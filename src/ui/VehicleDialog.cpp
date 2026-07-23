#include "VehicleDialog.h"
#include "database/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDateTime>

VehicleDialog::VehicleDialog(QWidget* parent, int vehicleId)
    : QDialog(parent), m_vehicleId(vehicleId), m_editMode(vehicleId > 0)
{
    buildUI();
    loadSiteCombo();
    if (m_editMode) loadVehicleData();
}

void VehicleDialog::buildUI()
{
    setWindowTitle(m_editMode ? "Edit Vehicle" : "New Vehicle Entry");
    setMinimumSize(480, 500);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel(m_editMode ? "EDIT VEHICLE" : "NEW VEHICLE ENTRY");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* formGroup = new QGroupBox("Vehicle Details");
    auto* form = new QFormLayout(formGroup);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_siteCombo = new QComboBox;
    m_siteCombo->addItem("-- Select Site --", 0);
    form->addRow("Site *:", m_siteCombo);

    m_vehicleNo = new QLineEdit;
    m_vehicleNo->setPlaceholderText("e.g. MH 02 AB 1234");
    form->addRow("Vehicle No. *:", m_vehicleNo);

    m_vehicleType = new QComboBox;
    m_vehicleType->setEditable(true);
    m_vehicleType->addItems({"Car", "SUV", "Truck", "Van", "Bike", "Auto", "Bus", "Other"});
    form->addRow("Vehicle Type:", m_vehicleType);

    m_driverName = new QLineEdit;
    m_driverName->setPlaceholderText("Driver name");
    form->addRow("Driver Name:", m_driverName);

    m_driverMobile = new QLineEdit;
    m_driverMobile->setPlaceholderText("Driver mobile");
    form->addRow("Driver Mobile:", m_driverMobile);

    m_purposeEdit = new QLineEdit;
    m_purposeEdit->setPlaceholderText("Purpose of visit");
    form->addRow("Purpose:", m_purposeEdit);

    m_entryTime = new QTimeEdit;
    m_entryTime->setDisplayFormat("HH:mm");
    m_entryTime->setTime(QTime::currentTime());
    form->addRow("Entry Time *:", m_entryTime);

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
    connect(saveBtn, &QPushButton::clicked, this, &VehicleDialog::saveVehicle);
    btnRow->addWidget(saveBtn);

    mainLayout->addLayout(btnRow);
}

void VehicleDialog::loadSiteCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT id, site_name FROM Sites WHERE status = 'Active' ORDER BY site_name");
    while (q.next()) {
        m_siteCombo->addItem(q.value("site_name").toString(), q.value("id").toInt());
    }
}

void VehicleDialog::loadVehicleData()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT * FROM Vehicles WHERE id = :id", {{":id", m_vehicleId}});
    if (!q.next()) return;

    int siteId = q.value("site_id").toInt();
    for (int i = 0; i < m_siteCombo->count(); ++i) {
        if (m_siteCombo->itemData(i).toInt() == siteId) {
            m_siteCombo->setCurrentIndex(i); break;
        }
    }
    m_vehicleNo->setText(q.value("vehicle_no").toString());
    m_vehicleType->setCurrentText(q.value("vehicle_type").toString());
    m_driverName->setText(q.value("driver_name").toString());
    m_driverMobile->setText(q.value("driver_mobile").toString());
    m_purposeEdit->setText(q.value("purpose").toString());

    QString entryStr = q.value("entry_time").toString();
    QTime et = QTime::fromString(entryStr, "HH:mm");
    if (!et.isValid()) et = QTime::fromString(entryStr, "yyyy-MM-dd HH:mm");
    if (et.isValid()) m_entryTime->setTime(et);

    m_notes->setPlainText(q.value("notes").toString());
}

void VehicleDialog::saveVehicle()
{
    if (m_siteCombo->currentData().toInt() == 0) {
        m_errorLabel->setText("Please select a site."); m_errorLabel->show(); return;
    }
    if (m_vehicleNo->text().trimmed().isEmpty()) {
        m_errorLabel->setText("Vehicle number is required."); m_errorLabel->show(); return;
    }
    m_errorLabel->hide();

    auto& db = DatabaseManager::instance();
    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    QString entryTime = today + " " + m_entryTime->time().toString("HH:mm");

    QVariantMap data;
    data[":sid"]    = m_siteCombo->currentData().toInt();
    data[":vno"]    = m_vehicleNo->text().trimmed().toUpper();
    data[":vtype"]  = m_vehicleType->currentText();
    data[":driver"] = m_driverName->text().trimmed();
    data[":dmobile"] = m_driverMobile->text().trimmed();
    data[":purpose"] = m_purposeEdit->text().trimmed();
    data[":entry"]  = entryTime;
    data[":notes"]  = m_notes->toPlainText().trimmed();

    bool ok;
    if (m_editMode) {
        data[":id"] = m_vehicleId;
        ok = db.executeNonQuery(
            "UPDATE Vehicles SET site_id = :sid, vehicle_no = :vno, vehicle_type = :vtype, "
            "driver_name = :driver, driver_mobile = :dmobile, purpose = :purpose, "
            "entry_time = :entry, notes = :notes WHERE id = :id", data
        );
    } else {
        ok = db.executeNonQuery(
            "INSERT INTO Vehicles (site_id, vehicle_no, vehicle_type, driver_name, "
            "driver_mobile, purpose, entry_time, notes) "
            "VALUES (:sid, :vno, :vtype, :driver, :dmobile, :purpose, :entry, :notes)", data
        );
    }

    if (ok) accept();
    else { m_errorLabel->setText("Failed to save vehicle entry."); m_errorLabel->show(); }
}
