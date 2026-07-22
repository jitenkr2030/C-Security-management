#include "LeaveDialog.h"
#include "database/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>

LeaveDialog::LeaveDialog(QWidget* parent, int leaveId, int userId)
    : QDialog(parent), m_leaveId(leaveId), m_userId(userId), m_editMode(leaveId > 0)
{
    buildUI();
    setWindowTitle(m_editMode ? "Edit Leave Request" : "Apply for Leave");
    setMinimumSize(500, 480);
    setModal(true);

    loadGuardCombo();

    if (m_editMode) {
        loadLeaveData();
    }
}

void LeaveDialog::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel(m_editMode ? "EDIT LEAVE" : "APPLY FOR LEAVE");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    // Form
    auto* formGroup = new QGroupBox("Leave Details");
    auto* form = new QFormLayout(formGroup);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_guardCombo = new QComboBox;
    m_guardCombo->addItem("-- Select Guard --", 0);
    connect(m_guardCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        // Show balance when guard is selected
        int gid = m_guardCombo->currentData().toInt();
        if (gid > 0) {
            auto& db = DatabaseManager::instance();
            auto sq = db.execute(
                "SELECT value FROM Settings WHERE key IN ('casual_leave','sick_leave','earned_leave') ORDER BY key"
            );
            int casual = 12, sick = 7, earned = 15;
            if (sq.next()) casual = sq.value("value").toInt();
            if (sq.next()) sick = sq.value("value").toInt();
            if (sq.next()) earned = sq.value("value").toInt();

            // Count used leaves this year
            int year = QDate::currentDate().year();
            auto used = db.execute(
                "SELECT leave_type, SUM(days) AS used_days FROM LeaveRecord "
                "WHERE guard_id = :gid AND status = 'Approved' "
                "AND strftime('%Y', start_date) = :year "
                "GROUP BY leave_type",
                {{":gid", gid}, {":year", QString::number(year)}}
            );
            int usedCasual = 0, usedSick = 0, usedEarned = 0;
            while (used.next()) {
                QString lt = used.value("leave_type").toString();
                int ud = used.value("used_days").toInt();
                if (lt == "Casual")      usedCasual = ud;
                else if (lt == "Sick")   usedSick = ud;
                else if (lt == "Earned") usedEarned = ud;
            }

            m_balanceLabel->setText(
                QString("Balance: Casual %1/%2 | Sick %3/%4 | Earned %5/%6")
                    .arg(casual - usedCasual).arg(casual)
                    .arg(sick - usedSick).arg(sick)
                    .arg(earned - usedEarned).arg(earned)
            );
        } else {
            m_balanceLabel->setText("Select a guard to see leave balance");
        }
    });
    form->addRow("Guard *:", m_guardCombo);

    m_balanceLabel = new QLabel("Select a guard to see leave balance");
    m_balanceLabel->setStyleSheet("color: #555E6B; font-size: 11px; font-style: italic;");
    form->addRow("", m_balanceLabel);

    m_leaveType = new QComboBox;
    m_leaveType->addItems({"Casual", "Sick", "Earned", "Compensatory", "Maternity", "Paternity", "Without Pay"});
    form->addRow("Leave Type *:", m_leaveType);

    m_startDate = new QDateEdit;
    m_startDate->setCalendarPopup(true);
    m_startDate->setDisplayFormat("yyyy-MM-dd");
    m_startDate->setDate(QDate::currentDate());
    connect(m_startDate, &QDateEdit::dateChanged, this, &LeaveDialog::updateDayCount);
    form->addRow("Start Date *:", m_startDate);

    m_endDate = new QDateEdit;
    m_endDate->setCalendarPopup(true);
    m_endDate->setDisplayFormat("yyyy-MM-dd");
    m_endDate->setDate(QDate::currentDate());
    connect(m_endDate, &QDateEdit::dateChanged, this, &LeaveDialog::updateDayCount);
    form->addRow("End Date *:", m_endDate);

    m_dayCount = new QSpinBox;
    m_dayCount->setRange(1, 365);
    m_dayCount->setValue(1);
    form->addRow("Days:", m_dayCount);

    m_reason = new QTextEdit;
    m_reason->setPlaceholderText("Reason for leave...");
    m_reason->setMaximumHeight(80);
    form->addRow("Reason:", m_reason);

    mainLayout->addWidget(formGroup);
    mainLayout->addStretch();

    // Buttons
    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();

    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("SecondaryButton");
    cancelBtn->setFixedSize(100, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);

    auto* saveBtn = new QPushButton(m_editMode ? "Update" : "Submit Request");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(150, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &LeaveDialog::saveLeave);
    btnRow->addWidget(saveBtn);

    mainLayout->addLayout(btnRow);
}

void LeaveDialog::loadGuardCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute(
        "SELECT id, guard_code, full_name FROM Guards "
        "WHERE status = 'Active' ORDER BY full_name"
    );
    while (q.next()) {
        m_guardCombo->addItem(
            q.value("full_name").toString() + " (" + q.value("guard_code").toString() + ")",
            q.value("id").toInt()
        );
    }
}

void LeaveDialog::loadLeaveData()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT * FROM LeaveRecord WHERE id = :id", {{":id", m_leaveId}});
    if (!q.next()) return;

    int guardId = q.value("guard_id").toInt();
    for (int i = 0; i < m_guardCombo->count(); ++i) {
        if (m_guardCombo->itemData(i).toInt() == guardId) {
            m_guardCombo->setCurrentIndex(i); break;
        }
    }
    m_leaveType->setCurrentText(q.value("leave_type").toString());
    m_startDate->setDate(QDate::fromString(q.value("start_date").toString(), "yyyy-MM-dd"));
    m_endDate->setDate(QDate::fromString(q.value("end_date").toString(), "yyyy-MM-dd"));
    m_dayCount->setValue(q.value("days").toInt());
    m_reason->setPlainText(q.value("reason").toString());
}

void LeaveDialog::updateDayCount()
{
    int days = m_startDate->date().daysTo(m_endDate->date()) + 1;
    if (days < 1) days = 1;
    m_dayCount->setValue(days);
}

void LeaveDialog::saveLeave()
{
    if (m_guardCombo->currentData().toInt() == 0) {
        m_errorLabel->setText("Please select a guard.");
        m_errorLabel->show();
        return;
    }
    if (m_startDate->date() > m_endDate->date()) {
        m_errorLabel->setText("End date must be after start date.");
        m_errorLabel->show();
        return;
    }
    m_errorLabel->hide();

    auto& db = DatabaseManager::instance();
    QVariantMap data;
    data[":gid"]    = m_guardCombo->currentData().toInt();
    data[":type"]   = m_leaveType->currentText();
    data[":start"]  = m_startDate->date().toString("yyyy-MM-dd");
    data[":end"]    = m_endDate->date().toString("yyyy-MM-dd");
    data[":days"]   = m_dayCount->value();
    data[":reason"] = m_reason->toPlainText();

    bool ok;
    if (m_editMode) {
        data[":id"] = m_leaveId;
        ok = db.executeNonQuery(
            "UPDATE LeaveRecord SET guard_id = :gid, leave_type = :type, "
            "start_date = :start, end_date = :end, days = :days, reason = :reason "
            "WHERE id = :id", data
        );
    } else {
        ok = db.executeNonQuery(
            "INSERT INTO LeaveRecord (guard_id, leave_type, start_date, end_date, days, reason) "
            "VALUES (:gid, :type, :start, :end, :days, :reason)", data
        );
    }

    if (ok) accept();
    else { m_errorLabel->setText("Failed to save leave request."); m_errorLabel->show(); }
}
