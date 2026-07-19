#include "DutyDialog.h"
#include "database/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QFrame>

DutyDialog::DutyDialog(QWidget* parent, int dutyId)
    : QDialog(parent), m_dutyId(dutyId), m_editMode(dutyId > 0)
{
    buildUI();
    setWindowTitle(m_editMode ? "Edit Duty Assignment" : "New Duty Assignment");
    setMinimumSize(520, 500);
    setModal(true);

    loadGuardCombo();
    loadSiteCombo();

    if (m_editMode) {
        loadDutyData();
    }
}

void DutyDialog::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel(m_editMode ? "EDIT DUTY" : "NEW DUTY ASSIGNMENT");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    // ---- Assignment ----
    auto* assignGroup = new QGroupBox("Assignment Details");
    auto* assignForm = new QFormLayout(assignGroup);
    assignForm->setSpacing(10);
    assignForm->setLabelAlignment(Qt::AlignRight);

    m_guardCombo = new QComboBox;
    m_guardCombo->addItem("-- Select Guard --", 0);
    assignForm->addRow("Guard *:", m_guardCombo);

    m_siteCombo = new QComboBox;
    m_siteCombo->addItem("-- Select Site --", 0);
    assignForm->addRow("Site *:", m_siteCombo);

    m_shiftCombo = new QComboBox;
    m_shiftCombo->addItems({"Morning", "Afternoon", "Night", "General"});
    assignForm->addRow("Shift *:", m_shiftCombo);

    m_dutyTypeCombo = new QComboBox;
    m_dutyTypeCombo->addItems({
        "Regular", "Temporary Transfer", "Emergency",
        "Overtime", "Replacement", "Training Duty"
    });
    assignForm->addRow("Duty Type:", m_dutyTypeCombo);

    mainLayout->addWidget(assignGroup);

    // ---- Schedule ----
    auto* schedGroup = new QGroupBox("Schedule");
    auto* schedForm = new QFormLayout(schedGroup);
    schedForm->setSpacing(10);
    schedForm->setLabelAlignment(Qt::AlignRight);

    m_startDate = new QDateEdit;
    m_startDate->setCalendarPopup(true);
    m_startDate->setDisplayFormat("yyyy-MM-dd");
    m_startDate->setDate(QDate::currentDate());
    schedForm->addRow("Start Date *:", m_startDate);

    m_endDate = new QDateEdit;
    m_endDate->setCalendarPopup(true);
    m_endDate->setDisplayFormat("yyyy-MM-dd");
    m_endDate->setDate(QDate::currentDate().addMonths(1));
    schedForm->addRow("End Date:", m_endDate);

    m_permanentCheck = new QCheckBox("Permanent assignment (no end date)");
    connect(m_permanentCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_endDate->setEnabled(!checked);
    });
    schedForm->addRow("", m_permanentCheck);

    mainLayout->addWidget(schedGroup);

    // ---- Notes ----
    auto* notesGroup = new QGroupBox("Notes");
    auto* notesLayout = new QVBoxLayout(notesGroup);
    m_notes = new QTextEdit;
    m_notes->setPlaceholderText("Special instructions, reason for transfer/emergency...");
    m_notes->setMaximumHeight(80);
    notesLayout->addWidget(m_notes);
    mainLayout->addWidget(notesGroup);

    mainLayout->addStretch();

    // ---- Buttons ----
    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();

    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("SecondaryButton");
    cancelBtn->setFixedSize(100, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);

    auto* saveBtn = new QPushButton(m_editMode ? "Update" : "Assign Duty");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(140, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &DutyDialog::saveDuty);
    btnRow->addWidget(saveBtn);

    mainLayout->addLayout(btnRow);
}

void DutyDialog::loadGuardCombo()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT id, guard_code, full_name, "
        "COALESCE((SELECT site_name FROM Sites WHERE id = Guards.site_id), 'Unassigned') "
        "AS current_site "
        "FROM Guards WHERE status = 'Active' ORDER BY full_name"
    );
    while (query.next()) {
        m_guardCombo->addItem(
            QString("%1 (%2) - %3")
                .arg(query.value("full_name").toString(),
                     query.value("guard_code").toString(),
                     query.value("current_site").toString()),
            query.value("id").toInt()
        );
    }
}

void DutyDialog::loadSiteCombo()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT s.id, s.site_name, c.company_name "
        "FROM Sites s LEFT JOIN Clients c ON s.client_id = c.id "
        "WHERE s.status = 'Active' ORDER BY s.site_name"
    );
    while (query.next()) {
        m_siteCombo->addItem(
            QString("%1 (%2)")
                .arg(query.value("site_name").toString(),
                     query.value("company_name").toString()),
            query.value("id").toInt()
        );
    }
}

void DutyDialog::loadDutyData()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute("SELECT * FROM Duty WHERE id = :id", {{":id", m_dutyId}});
    if (!query.next()) return;

    int guardId = query.value("guard_id").toInt();
    int siteId  = query.value("site_id").toInt();

    for (int i = 0; i < m_guardCombo->count(); ++i) {
        if (m_guardCombo->itemData(i).toInt() == guardId) {
            m_guardCombo->setCurrentIndex(i);
            break;
        }
    }
    for (int i = 0; i < m_siteCombo->count(); ++i) {
        if (m_siteCombo->itemData(i).toInt() == siteId) {
            m_siteCombo->setCurrentIndex(i);
            break;
        }
    }

    m_shiftCombo->setCurrentText(query.value("shift").toString());
    m_startDate->setDate(QDate::fromString(query.value("start_date").toString(), "yyyy-MM-dd"));

    QString endDateStr = query.value("end_date").toString();
    if (endDateStr.isEmpty()) {
        m_permanentCheck->setChecked(true);
    } else {
        m_endDate->setDate(QDate::fromString(endDateStr, "yyyy-MM-dd"));
    }

    m_notes->setPlainText(query.value("notes").toString());
}

bool DutyDialog::validate()
{
    if (m_guardCombo->currentData().toInt() == 0) {
        m_errorLabel->setText("Please select a guard.");
        m_errorLabel->show();
        m_guardCombo->setFocus();
        return false;
    }
    if (m_siteCombo->currentData().toInt() == 0) {
        m_errorLabel->setText("Please select a site.");
        m_errorLabel->show();
        m_siteCombo->setFocus();
        return false;
    }
    m_errorLabel->hide();
    return true;
}

void DutyDialog::saveDuty()
{
    if (!validate()) return;

    int guardId  = m_guardCombo->currentData().toInt();
    int siteId   = m_siteCombo->currentData().toInt();
    QString shift = m_shiftCombo->currentText();
    QString startDate = m_startDate->date().toString("yyyy-MM-dd");
    QString endDate   = m_permanentCheck->isChecked()
                        ? "" : m_endDate->date().toString("yyyy-MM-dd");
    int permanent = m_permanentCheck->isChecked() ? 1 : 0;
    QString notes = m_notes->toPlainText();

    auto& db = DatabaseManager::instance();
    bool success;

    if (m_editMode) {
        success = db.executeNonQuery(
            "UPDATE Duty SET "
            "guard_id = :gid, site_id = :sid, shift = :shift, "
            "start_date = :start, end_date = :end, "
            "is_permanent = :perm, notes = :notes "
            "WHERE id = :id",
            {
                {":gid",   guardId}, {":sid", siteId},
                {":shift", shift},   {":start", startDate},
                {":end",   endDate.isEmpty() ? QVariant(QVariant::String) : endDate},
                {":perm",  permanent}, {":notes", notes},
                {":id",    m_dutyId}
            }
        );
    } else {
        success = db.executeNonQuery(
            "INSERT INTO Duty (guard_id, site_id, shift, start_date, end_date, is_permanent, notes) "
            "VALUES (:gid, :sid, :shift, :start, :end, :perm, :notes)",
            {
                {":gid",   guardId}, {":sid", siteId},
                {":shift", shift},   {":start", startDate},
                {":end",   endDate.isEmpty() ? QVariant(QVariant::String) : endDate},
                {":perm",  permanent}, {":notes", notes}
            }
        );
    }

    if (success) {
        // Also update the guard's current site assignment
        db.executeNonQuery(
            "UPDATE Guards SET site_id = :sid, client_id = "
            "(SELECT client_id FROM Sites WHERE id = :sid2), "
            "updated_at = datetime('now','localtime') "
            "WHERE id = :gid",
            {{":sid", siteId}, {":sid2", siteId}, {":gid", guardId}}
        );
        accept();
    } else {
        m_errorLabel->setText("Failed to save duty assignment.");
        m_errorLabel->show();
    }
}
