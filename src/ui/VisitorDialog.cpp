#include "VisitorDialog.h"
#include "database/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDateTime>

VisitorDialog::VisitorDialog(QWidget* parent, int visitorId)
    : QDialog(parent), m_visitorId(visitorId), m_editMode(visitorId > 0)
{
    buildUI();
    loadSiteCombo();
    if (m_editMode) loadVisitorData();
}

void VisitorDialog::buildUI()
{
    setWindowTitle(m_editMode ? "Edit Visitor" : "New Visitor Entry");
    setMinimumSize(500, 560);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel(m_editMode ? "EDIT VISITOR" : "NEW VISITOR ENTRY");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* formGroup = new QGroupBox("Visitor Details");
    auto* form = new QFormLayout(formGroup);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_siteCombo = new QComboBox;
    m_siteCombo->addItem("-- Select Site --", 0);
    form->addRow("Site *:", m_siteCombo);

    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText("Visitor name");
    form->addRow("Name *:", m_nameEdit);

    m_mobileEdit = new QLineEdit;
    m_mobileEdit->setPlaceholderText("Mobile number");
    form->addRow("Mobile:", m_mobileEdit);

    m_idProofCombo = new QComboBox;
    m_idProofCombo->addItems({"Aadhaar Card", "PAN Card", "Driving License",
                              "Voter ID", "Passport", "Employee ID", "Other"});
    form->addRow("ID Proof:", m_idProofCombo);

    m_idNumberEdit = new QLineEdit;
    m_idNumberEdit->setPlaceholderText("ID number");
    form->addRow("ID Number:", m_idNumberEdit);

    m_purposeEdit = new QLineEdit;
    m_purposeEdit->setPlaceholderText("Purpose of visit");
    form->addRow("Purpose:", m_purposeEdit);

    m_whomToMeet = new QLineEdit;
    m_whomToMeet->setPlaceholderText("Person to meet");
    form->addRow("To Meet:", m_whomToMeet);

    m_entryTime = new QTimeEdit;
    m_entryTime->setDisplayFormat("HH:mm");
    m_entryTime->setTime(QTime::currentTime());
    form->addRow("Entry Time *:", m_entryTime);

    m_vehicleNo = new QLineEdit;
    m_vehicleNo->setPlaceholderText("Vehicle number (if any)");
    form->addRow("Vehicle No:", m_vehicleNo);

    m_notes = new QTextEdit;
    m_notes->setPlaceholderText("Notes...");
    m_notes->setMaximumHeight(60);
    form->addRow("Notes:", m_notes);

    mainLayout->addWidget(formGroup);
    mainLayout->addStretch();

    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();

    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("SecondaryButton");
    cancelBtn->setFixedSize(100, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);

    auto* saveBtn = new QPushButton(m_editMode ? "Update" : "Register Entry");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(150, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &VisitorDialog::saveVisitor);
    btnRow->addWidget(saveBtn);

    mainLayout->addLayout(btnRow);
}

void VisitorDialog::loadSiteCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT id, site_name FROM Sites WHERE status = 'Active' ORDER BY site_name");
    while (q.next()) {
        m_siteCombo->addItem(q.value("site_name").toString(), q.value("id").toInt());
    }
}

void VisitorDialog::loadVisitorData()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT * FROM Visitors WHERE id = :id", {{":id", m_visitorId}});
    if (!q.next()) return;

    int siteId = q.value("site_id").toInt();
    for (int i = 0; i < m_siteCombo->count(); ++i) {
        if (m_siteCombo->itemData(i).toInt() == siteId) {
            m_siteCombo->setCurrentIndex(i); break;
        }
    }
    m_nameEdit->setText(q.value("name").toString());
    m_mobileEdit->setText(q.value("mobile").toString());
    m_idProofCombo->setCurrentText(q.value("id_proof").toString());
    m_idNumberEdit->setText(q.value("id_number").toString());
    m_purposeEdit->setText(q.value("purpose").toString());
    m_whomToMeet->setText(q.value("whom_to_meet").toString());

    QString entryStr = q.value("entry_time").toString();
    QTime et = QTime::fromString(entryStr, "HH:mm");
    if (!et.isValid()) et = QTime::fromString(entryStr, "yyyy-MM-dd HH:mm");
    if (et.isValid()) m_entryTime->setTime(et);

    m_vehicleNo->setText(q.value("vehicle_no").toString());
    m_notes->setPlainText(q.value("notes").toString());
}

void VisitorDialog::saveVisitor()
{
    if (m_siteCombo->currentData().toInt() == 0) {
        m_errorLabel->setText("Please select a site."); m_errorLabel->show(); return;
    }
    if (m_nameEdit->text().trimmed().isEmpty()) {
        m_errorLabel->setText("Visitor name is required."); m_errorLabel->show(); return;
    }
    m_errorLabel->hide();

    auto& db = DatabaseManager::instance();
    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    QString entryTime = today + " " + m_entryTime->time().toString("HH:mm");

    QVariantMap data;
    data[":sid"]     = m_siteCombo->currentData().toInt();
    data[":name"]    = m_nameEdit->text().trimmed();
    data[":mobile"]  = m_mobileEdit->text().trimmed();
    data[":idproof"] = m_idProofCombo->currentText();
    data[":idnum"]   = m_idNumberEdit->text().trimmed();
    data[":purpose"] = m_purposeEdit->text().trimmed();
    data[":meet"]    = m_whomToMeet->text().trimmed();
    data[":entry"]   = entryTime;
    data[":veh"]     = m_vehicleNo->text().trimmed();
    data[":notes"]   = m_notes->toPlainText().trimmed();

    bool ok;
    if (m_editMode) {
        data[":id"] = m_visitorId;
        ok = db.executeNonQuery(
            "UPDATE Visitors SET site_id = :sid, name = :name, mobile = :mobile, "
            "id_proof = :idproof, id_number = :idnum, purpose = :purpose, "
            "whom_to_meet = :meet, entry_time = :entry, vehicle_no = :veh, notes = :notes "
            "WHERE id = :id", data
        );
    } else {
        ok = db.executeNonQuery(
            "INSERT INTO Visitors (site_id, name, mobile, id_proof, id_number, purpose, "
            "whom_to_meet, entry_time, vehicle_no, notes) "
            "VALUES (:sid, :name, :mobile, :idproof, :idnum, :purpose, :meet, :entry, :veh, :notes)",
            data
        );
    }

    if (ok) accept();
    else { m_errorLabel->setText("Failed to save visitor entry."); m_errorLabel->show(); }
}
