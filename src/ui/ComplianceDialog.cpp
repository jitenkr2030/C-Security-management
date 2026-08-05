#include "ComplianceDialog.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>

ComplianceDialog::ComplianceDialog(QWidget* parent, Type type, int recordId)
    : QDialog(parent), m_type(type), m_recordId(recordId), m_editMode(recordId > 0)
{
    switch (type) {
        case FilingType: buildFilingUI(); if (m_editMode) loadFilingData(); break;
        case LicenseType: buildLicenseUI(); if (m_editMode) loadLicenseData(); break;
        case MinWageType: buildMinWageUI(); if (m_editMode) loadMinWageData(); break;
    }
}

void ComplianceDialog::buildFilingUI()
{
    setWindowTitle(m_editMode ? "Edit Filing" : "Add Filing Record");
    setMinimumSize(480, 460);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(14);

    auto* titleLabel = new QLabel(m_editMode ? "EDIT FILING" : "NEW COMPLIANCE FILING");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* formGroup = new QGroupBox("Filing Details");
    auto* form = new QFormLayout(formGroup);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_filingArea = new QComboBox;
    m_filingArea->addItems({"EPF", "ESIC", "Professional Tax", "Bonus", "Gratuity",
                            "POSH", "Shops & Establishments", "PSARA", "Contract Labour", "Other"});
    form->addRow("Compliance Area:", m_filingArea);

    m_filingType = new QComboBox;
    m_filingType->setEditable(true);
    m_filingType->addItems({"Monthly Return", "ECR Filing", "Challan Payment",
                            "Half-Yearly Return", "Annual Return", "Renewal",
                            "Registration", "Quarterly Filing"});
    form->addRow("Filing Type:", m_filingType);

    m_filingPeriod = new QLineEdit;
    m_filingPeriod->setPlaceholderText("e.g. 2025-04 or Apr-Sep 2025");
    form->addRow("Period:", m_filingPeriod);

    m_filingDueDate = new QDateEdit;
    m_filingDueDate->setCalendarPopup(true);
    m_filingDueDate->setDisplayFormat("yyyy-MM-dd");
    form->addRow("Due Date *:", m_filingDueDate);

    m_filingFiledDate = new QDateEdit;
    m_filingFiledDate->setCalendarPopup(true);
    m_filingFiledDate->setDisplayFormat("yyyy-MM-dd");
    m_filingFiledDate->setDate(QDate::currentDate());
    form->addRow("Filed Date:", m_filingFiledDate);

    m_filingAmount = new QDoubleSpinBox;
    m_filingAmount->setRange(0, 10000000);
    m_filingAmount->setDecimals(0);
    m_filingAmount->setPrefix("Rs. ");
    form->addRow("Amount:", m_filingAmount);

    m_filingStatus = new QComboBox;
    m_filingStatus->addItems({"Pending", "Filed", "Late", "Overdue"});
    form->addRow("Status:", m_filingStatus);

    auto* challanRow = new QHBoxLayout;
    m_filingChallanPath = new QLineEdit;
    m_filingChallanPath->setPlaceholderText("Challan/receipt file path");
    challanRow->addWidget(m_filingChallanPath, 1);
    auto* browseBtn = new QPushButton("Browse");
    browseBtn->setObjectName("SecondaryButton");
    browseBtn->setFixedSize(80, 30);
    browseBtn->setCursor(Qt::PointingHandCursor);
    connect(browseBtn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getOpenFileName(this, "Select File");
        if (!path.isEmpty()) m_filingChallanPath->setText(path);
    });
    challanRow->addWidget(browseBtn);
    form->addRow("Challan:", challanRow);

    m_filingNotes = new QTextEdit;
    m_filingNotes->setPlaceholderText("Notes...");
    m_filingNotes->setMaximumHeight(50);
    form->addRow("Notes:", m_filingNotes);

    mainLayout->addWidget(formGroup, 1);

    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("SecondaryButton");
    cancelBtn->setFixedSize(100, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);
    auto* saveBtn = new QPushButton(m_editMode ? "Update" : "Save");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(100, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &ComplianceDialog::saveRecord);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void ComplianceDialog::buildLicenseUI()
{
    setWindowTitle(m_editMode ? "Edit License" : "Add License");
    setMinimumSize(500, 480);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(14);

    auto* titleLabel = new QLabel(m_editMode ? "EDIT LICENSE" : "NEW LICENSE / REGISTRATION");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* formGroup = new QGroupBox("License Details");
    auto* form = new QFormLayout(formGroup);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_licenseType = new QComboBox;
    m_licenseType->setEditable(true);
    m_licenseType->addItems({"PSARA License", "Contractor License", "EPF Registration",
                             "ESIC Registration", "PT Registration", "Shops & Establishments",
                             "Trade License", "Fire NOC", "Police Verification", "Other"});
    form->addRow("License Type *:", m_licenseType);

    m_licenseNumber = new QLineEdit;
    m_licenseNumber->setPlaceholderText("License/Registration number");
    form->addRow("License Number *:", m_licenseNumber);

    m_issuingAuthority = new QLineEdit;
    m_issuingAuthority->setPlaceholderText("Issuing authority name");
    form->addRow("Issuing Authority:", m_issuingAuthority);

    m_licenseState = new QLineEdit;
    m_licenseState->setPlaceholderText("State of operation");
    form->addRow("State:", m_licenseState);

    m_issueDate = new QDateEdit;
    m_issueDate->setCalendarPopup(true);
    m_issueDate->setDisplayFormat("yyyy-MM-dd");
    form->addRow("Issue Date:", m_issueDate);

    m_expiryDate = new QDateEdit;
    m_expiryDate->setCalendarPopup(true);
    m_expiryDate->setDisplayFormat("yyyy-MM-dd");
    m_expiryDate->setDate(QDate::currentDate().addYears(1));
    form->addRow("Expiry Date *:", m_expiryDate);

    m_renewalStatus = new QComboBox;
    m_renewalStatus->addItems({"Active", "Under Renewal", "Expired", "Suspended", "Revoked"});
    form->addRow("Status:", m_renewalStatus);

    auto* docRow = new QHBoxLayout;
    m_docPath = new QLineEdit;
    m_docPath->setPlaceholderText("Document file path");
    docRow->addWidget(m_docPath, 1);
    auto* browseBtn = new QPushButton("Browse");
    browseBtn->setObjectName("SecondaryButton");
    browseBtn->setFixedSize(80, 30);
    browseBtn->setCursor(Qt::PointingHandCursor);
    connect(browseBtn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getOpenFileName(this, "Select Document");
        if (!path.isEmpty()) m_docPath->setText(path);
    });
    docRow->addWidget(browseBtn);
    form->addRow("Document:", docRow);

    m_licenseNotes = new QTextEdit;
    m_licenseNotes->setPlaceholderText("Notes...");
    m_licenseNotes->setMaximumHeight(50);
    form->addRow("Notes:", m_licenseNotes);

    m_licenseStatus = new QComboBox;
    m_licenseStatus->addItems({"Active", "Expired", "Under Renewal", "Suspended"});
    form->addRow("Current Status:", m_licenseStatus);

    mainLayout->addWidget(formGroup, 1);

    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("SecondaryButton");
    cancelBtn->setFixedSize(100, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);
    auto* saveBtn = new QPushButton(m_editMode ? "Update" : "Save");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(100, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &ComplianceDialog::saveRecord);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void ComplianceDialog::buildMinWageUI()
{
    setWindowTitle(m_editMode ? "Edit Minimum Wage" : "Add Minimum Wage Rate");
    setMinimumSize(460, 400);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(14);

    auto* titleLabel = new QLabel(m_editMode ? "EDIT MINIMUM WAGE" : "ADD MINIMUM WAGE RATE");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* formGroup = new QGroupBox("Wage Details");
    auto* form = new QFormLayout(formGroup);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_wageState = new QLineEdit;
    m_wageState->setPlaceholderText("e.g. Maharashtra");
    form->addRow("State *:", m_wageState);

    m_wageZone = new QComboBox;
    m_wageZone->addItems({"Zone I", "Zone II", "Zone III", "Zone IV"});
    form->addRow("Zone:", m_wageZone);

    m_wageSkill = new QComboBox;
    m_wageSkill->addItems({"Unskilled", "Semi-skilled", "Skilled", "Highly Skilled"});
    form->addRow("Skill Category:", m_wageSkill);

    m_basicWage = new QDoubleSpinBox;
    m_basicWage->setRange(0, 100000);
    m_basicWage->setDecimals(0);
    m_basicWage->setPrefix("Rs. ");
    m_basicWage->setSingleStep(100);
    connect(m_basicWage, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double) {
        m_totalWage->setValue(m_basicWage->value() + m_vda->value());
    });
    form->addRow("Basic Wage *:", m_basicWage);

    m_vda = new QDoubleSpinBox;
    m_vda->setRange(0, 50000);
    m_vda->setDecimals(0);
    m_vda->setPrefix("Rs. ");
    m_vda->setSingleStep(50);
    connect(m_vda, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double) {
        m_totalWage->setValue(m_basicWage->value() + m_vda->value());
    });
    form->addRow("VDA:", m_vda);

    m_totalWage = new QDoubleSpinBox;
    m_totalWage->setRange(0, 100000);
    m_totalWage->setDecimals(0);
    m_totalWage->setPrefix("Rs. ");
    m_totalWage->setReadOnly(true);
    m_totalWage->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_totalWage->setStyleSheet("QDoubleSpinBox { font-weight: bold; color: #D4B44C; }");
    form->addRow("Total Wage:", m_totalWage);

    m_effectiveFrom = new QDateEdit;
    m_effectiveFrom->setCalendarPopup(true);
    m_effectiveFrom->setDisplayFormat("yyyy-MM-dd");
    m_effectiveFrom->setDate(QDate::currentDate());
    form->addRow("Effective From:", m_effectiveFrom);

    m_notificationNo = new QLineEdit;
    m_notificationNo->setPlaceholderText("Government notification number");
    form->addRow("Notification No:", m_notificationNo);

    mainLayout->addWidget(formGroup, 1);

    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("SecondaryButton");
    cancelBtn->setFixedSize(100, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);
    auto* saveBtn = new QPushButton(m_editMode ? "Update" : "Save");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(100, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &ComplianceDialog::saveRecord);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void ComplianceDialog::loadFilingData()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT * FROM ComplianceFilings WHERE id = :id", {{":id", m_recordId}});
    if (!q.next()) return;
    m_filingArea->setCurrentText(q.value("compliance_area").toString());
    m_filingType->setCurrentText(q.value("filing_type").toString());
    m_filingPeriod->setText(q.value("filing_period").toString());
    m_filingDueDate->setDate(QDate::fromString(q.value("due_date").toString(), "yyyy-MM-dd"));
    m_filingFiledDate->setDate(QDate::fromString(q.value("filed_date").toString(), "yyyy-MM-dd"));
    m_filingAmount->setValue(q.value("amount").toDouble());
    m_filingStatus->setCurrentText(q.value("status").toString());
    m_filingChallanPath->setText(q.value("challan_path").toString());
    m_filingNotes->setPlainText(q.value("notes").toString());
}

void ComplianceDialog::loadLicenseData()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT * FROM ComplianceLicenses WHERE id = :id", {{":id", m_recordId}});
    if (!q.next()) return;
    m_licenseType->setCurrentText(q.value("license_type").toString());
    m_licenseNumber->setText(q.value("license_number").toString());
    m_issuingAuthority->setText(q.value("issuing_authority").toString());
    m_licenseState->setText(q.value("state").toString());
    m_issueDate->setDate(QDate::fromString(q.value("issue_date").toString(), "yyyy-MM-dd"));
    m_expiryDate->setDate(QDate::fromString(q.value("expiry_date").toString(), "yyyy-MM-dd"));
    m_renewalStatus->setCurrentText(q.value("renewal_status").toString());
    m_docPath->setText(q.value("document_path").toString());
    m_licenseNotes->setPlainText(q.value("notes").toString());
    m_licenseStatus->setCurrentText(q.value("status").toString());
}

void ComplianceDialog::loadMinWageData()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT * FROM MinWages WHERE id = :id", {{":id", m_recordId}});
    if (!q.next()) return;
    m_wageState->setText(q.value("state").toString());
    m_wageZone->setCurrentText(q.value("zone").toString());
    m_wageSkill->setCurrentText(q.value("skill_category").toString());
    m_basicWage->setValue(q.value("basic_wage").toDouble());
    m_vda->setValue(q.value("vda").toDouble());
    m_totalWage->setValue(q.value("total_wage").toDouble());
    m_effectiveFrom->setDate(QDate::fromString(q.value("effective_from").toString(), "yyyy-MM-dd"));
    m_notificationNo->setText(q.value("notification_no").toString());
}

void ComplianceDialog::saveRecord()
{
    auto& db = DatabaseManager::instance();
    bool ok;

    if (m_type == FilingType) {
        if (m_filingPeriod->text().trimmed().isEmpty()) { m_errorLabel->setText("Period is required."); m_errorLabel->show(); return; }
        m_errorLabel->hide();
        QVariantMap data;
        data[":area"] = m_filingArea->currentText();
        data[":type"] = m_filingType->currentText();
        data[":period"] = m_filingPeriod->text().trimmed();
        data[":due"] = m_filingDueDate->date().toString("yyyy-MM-dd");
        data[":filed"] = m_filingFiledDate->date().toString("yyyy-MM-dd");
        data[":amount"] = m_filingAmount->value();
        data[":status"] = m_filingStatus->currentText();
        data[":challan"] = m_filingChallanPath->text().trimmed();
        data[":notes"] = m_filingNotes->toPlainText().trimmed();
        if (m_editMode) {
            data[":id"] = m_recordId;
            ok = db.executeNonQuery("UPDATE ComplianceFilings SET compliance_area=:area, filing_type=:type, filing_period=:period, due_date=:due, filed_date=:filed, amount=:amount, status=:status, challan_path=:challan, notes=:notes WHERE id=:id", data);
        } else {
            ok = db.executeNonQuery("INSERT INTO ComplianceFilings (compliance_area, filing_type, filing_period, due_date, filed_date, amount, status, challan_path, notes) VALUES (:area, :type, :period, :due, :filed, :amount, :status, :challan, :notes)", data);
        }
    }
    else if (m_type == LicenseType) {
        if (m_licenseNumber->text().trimmed().isEmpty()) { m_errorLabel->setText("License number is required."); m_errorLabel->show(); return; }
        m_errorLabel->hide();
        QVariantMap data;
        data[":ltype"] = m_licenseType->currentText();
        data[":num"] = m_licenseNumber->text().trimmed();
        data[":auth"] = m_issuingAuthority->text().trimmed();
        data[":state"] = m_licenseState->text().trimmed();
        data[":issue"] = m_issueDate->date().toString("yyyy-MM-dd");
        data[":expiry"] = m_expiryDate->date().toString("yyyy-MM-dd");
        data[":renewal"] = m_renewalStatus->currentText();
        data[":doc"] = m_docPath->text().trimmed();
        data[":notes"] = m_licenseNotes->toPlainText().trimmed();
        data[":status"] = m_licenseStatus->currentText();
        if (m_editMode) {
            data[":id"] = m_recordId;
            ok = db.executeNonQuery("UPDATE ComplianceLicenses SET license_type=:ltype, license_number=:num, issuing_authority=:auth, state=:state, issue_date=:issue, expiry_date=:expiry, renewal_status=:renewal, document_path=:doc, notes=:notes, status=:status WHERE id=:id", data);
        } else {
            ok = db.executeNonQuery("INSERT INTO ComplianceLicenses (license_type, license_number, issuing_authority, state, issue_date, expiry_date, renewal_status, document_path, notes, status) VALUES (:ltype, :num, :auth, :state, :issue, :expiry, :renewal, :doc, :notes, :status)", data);
        }
    }
    else if (m_type == MinWageType) {
        if (m_wageState->text().trimmed().isEmpty()) { m_errorLabel->setText("State is required."); m_errorLabel->show(); return; }
        m_errorLabel->hide();
        QVariantMap data;
        data[":state"] = m_wageState->text().trimmed();
        data[":zone"] = m_wageZone->currentText();
        data[":skill"] = m_wageSkill->currentText();
        data[":basic"] = m_basicWage->value();
        data[":vda"] = m_vda->value();
        data[":total"] = m_totalWage->value();
        data[":from"] = m_effectiveFrom->date().toString("yyyy-MM-dd");
        data[":notif"] = m_notificationNo->text().trimmed();
        if (m_editMode) {
            data[":id"] = m_recordId;
            ok = db.executeNonQuery("UPDATE MinWages SET state=:state, zone=:zone, skill_category=:skill, basic_wage=:basic, vda=:vda, total_wage=:total, effective_from=:from, notification_no=:notif WHERE id=:id", data);
        } else {
            ok = db.executeNonQuery("INSERT INTO MinWages (state, zone, skill_category, basic_wage, vda, total_wage, effective_from, notification_no) VALUES (:state, :zone, :skill, :basic, :vda, :total, :from, :notif)", data);
        }
    }

    if (ok) accept();
    else { m_errorLabel->setText("Failed to save record."); m_errorLabel->show(); }
}
