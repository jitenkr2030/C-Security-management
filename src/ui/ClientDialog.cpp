#include "ClientDialog.h"
#include "database/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QFileDialog>
#include <QMessageBox>

ClientDialog::ClientDialog(QWidget* parent, int clientId)
    : QDialog(parent), m_clientId(clientId), m_editMode(clientId > 0)
{
    buildUI();
    setWindowTitle(m_editMode ? "Edit Client" : "Add New Client");
    setMinimumSize(650, 620);
    setModal(true);

    if (m_editMode) {
        loadClientData();
    }
}

void ClientDialog::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel(m_editMode ? "EDIT CLIENT" : "ADD NEW CLIENT");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* formContent = new QWidget;
    auto* formLayout = new QVBoxLayout(formContent);
    formLayout->setSpacing(20);

    // ---- Company Information ----
    auto* companyGroup = new QGroupBox("Company Information");
    auto* companyForm = new QFormLayout(companyGroup);
    companyForm->setSpacing(10);
    companyForm->setLabelAlignment(Qt::AlignRight);

    m_clientCode = new QLineEdit;
    m_clientCode->setPlaceholderText("e.g., CLI-001");
    companyForm->addRow("Client Code *:", m_clientCode);

    m_companyName = new QLineEdit;
    m_companyName->setPlaceholderText("Company or organization name");
    companyForm->addRow("Company Name *:", m_companyName);

    m_contactPerson = new QLineEdit;
    m_contactPerson->setPlaceholderText("Primary contact person");
    companyForm->addRow("Contact Person:", m_contactPerson);

    m_mobile = new QLineEdit;
    m_mobile->setPlaceholderText("Phone number");
    companyForm->addRow("Mobile *:", m_mobile);

    m_email = new QLineEdit;
    m_email->setPlaceholderText("Email address");
    companyForm->addRow("Email:", m_email);

    formLayout->addWidget(companyGroup);

    // ---- Address ----
    auto* addressGroup = new QGroupBox("Address");
    auto* addressForm = new QFormLayout(addressGroup);
    addressForm->setSpacing(10);
    addressForm->setLabelAlignment(Qt::AlignRight);

    m_address = new QLineEdit;
    m_address->setPlaceholderText("Full address");
    addressForm->addRow("Address:", m_address);

    m_city = new QLineEdit;
    m_city->setPlaceholderText("City");
    addressForm->addRow("City:", m_city);

    formLayout->addWidget(addressGroup);

    // ---- Billing & Contract ----
    auto* billingGroup = new QGroupBox("Billing & Contract");
    auto* billingForm = new QFormLayout(billingGroup);
    billingForm->setSpacing(10);
    billingForm->setLabelAlignment(Qt::AlignRight);

    m_gstNumber = new QLineEdit;
    m_gstNumber->setPlaceholderText("GST registration number");
    billingForm->addRow("GST Number:", m_gstNumber);

    m_billingRate = new QDoubleSpinBox;
    m_billingRate->setRange(0, 9999999);
    m_billingRate->setPrefix("Rs. ");
    m_billingRate->setDecimals(0);
    billingForm->addRow("Billing Rate (Per Guard/Month):", m_billingRate);

    m_contractStart = new QDateEdit;
    m_contractStart->setCalendarPopup(true);
    m_contractStart->setDisplayFormat("yyyy-MM-dd");
    m_contractStart->setDate(QDate::currentDate());
    billingForm->addRow("Contract Start:", m_contractStart);

    m_contractEnd = new QDateEdit;
    m_contractEnd->setCalendarPopup(true);
    m_contractEnd->setDisplayFormat("yyyy-MM-dd");
    m_contractEnd->setDate(QDate::currentDate().addYears(1));
    billingForm->addRow("Contract End:", m_contractEnd);

    auto* browseRow = new QHBoxLayout;
    m_agreementPath = new QLineEdit;
    m_agreementPath->setPlaceholderText("Path to agreement file");
    browseRow->addWidget(m_agreementPath);

    auto* browseBtn = new QPushButton("Browse");
    browseBtn->setObjectName("SecondaryButton");
    browseBtn->setFixedSize(80, 36);
    browseBtn->setCursor(Qt::PointingHandCursor);
    connect(browseBtn, &QPushButton::clicked, this, [this]() {
        QString file = QFileDialog::getOpenFileName(
            this, "Select Agreement File", QString(),
            "Documents (*.pdf *.doc *.docx *.jpg *.png);;All Files (*)"
        );
        if (!file.isEmpty()) {
            m_agreementPath->setText(file);
        }
    });
    browseRow->addWidget(browseBtn);
    billingForm->addRow("Agreement:", browseRow);

    m_invoiceTerms = new QTextEdit;
    m_invoiceTerms->setPlaceholderText("Invoice terms, payment cycle, special conditions...");
    m_invoiceTerms->setMaximumHeight(80);
    billingForm->addRow("Invoice Terms:", m_invoiceTerms);

    formLayout->addWidget(billingGroup);

    // ---- Status & Notes ----
    auto* statusGroup = new QGroupBox("Status");
    auto* statusForm = new QFormLayout(statusGroup);
    statusForm->setSpacing(10);
    statusForm->setLabelAlignment(Qt::AlignRight);

    m_status = new QComboBox;
    m_status->addItems({"Active", "Inactive", "Under Negotiation", "Contract Expired"});
    statusForm->addRow("Status:", m_status);

    m_notes = new QTextEdit;
    m_notes->setPlaceholderText("Additional notes about this client...");
    m_notes->setMaximumHeight(80);
    statusForm->addRow("Notes:", m_notes);

    formLayout->addWidget(statusGroup);

    scroll->setWidget(formContent);
    mainLayout->addWidget(scroll, 1);

    // ---- Buttons ----
    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();

    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("SecondaryButton");
    cancelBtn->setFixedSize(100, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);

    auto* saveBtn = new QPushButton(m_editMode ? "Update Client" : "Save Client");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(150, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &ClientDialog::saveClient);
    btnRow->addWidget(saveBtn);

    mainLayout->addLayout(btnRow);
}

void ClientDialog::loadClientData()
{
    auto& db = DatabaseManager::instance();
    QSqlQuery query = db.execute(
        "SELECT * FROM Clients WHERE id = :id",
        {{":id", m_clientId}}
    );

    if (!query.next()) return;

    m_clientCode->setText(query.value("client_code").toString());
    m_companyName->setText(query.value("company_name").toString());
    m_contactPerson->setText(query.value("contact_person").toString());
    m_mobile->setText(query.value("mobile").toString());
    m_email->setText(query.value("email").toString());
    m_address->setText(query.value("address").toString());
    m_city->setText(query.value("city").toString());
    m_gstNumber->setText(query.value("gst_number").toString());
    m_billingRate->setValue(query.value("billing_rate").toDouble());
    m_contractStart->setDate(QDate::fromString(query.value("contract_start").toString(), "yyyy-MM-dd"));
    m_contractEnd->setDate(QDate::fromString(query.value("contract_end").toString(), "yyyy-MM-dd"));
    m_agreementPath->setText(query.value("agreement_path").toString());
    m_invoiceTerms->setPlainText(query.value("invoice_terms").toString());
    m_status->setCurrentText(query.value("status").toString());
    m_notes->setPlainText(query.value("notes").toString());
}

bool ClientDialog::validate()
{
    if (m_clientCode->text().trimmed().isEmpty()) {
        m_errorLabel->setText("Client code is required.");
        m_errorLabel->show();
        m_clientCode->setFocus();
        return false;
    }
    if (m_companyName->text().trimmed().isEmpty()) {
        m_errorLabel->setText("Company name is required.");
        m_errorLabel->show();
        m_companyName->setFocus();
        return false;
    }
    if (m_mobile->text().trimmed().isEmpty()) {
        m_errorLabel->setText("Mobile number is required.");
        m_errorLabel->show();
        m_mobile->setFocus();
        return false;
    }

    m_errorLabel->hide();
    return true;
}

void ClientDialog::saveClient()
{
    if (!validate()) return;

    QVariantMap data;
    data[":client_code"]    = m_clientCode->text().trimmed();
    data[":company_name"]   = m_companyName->text().trimmed();
    data[":contact_person"] = m_contactPerson->text().trimmed();
    data[":mobile"]         = m_mobile->text().trimmed();
    data[":email"]          = m_email->text().trimmed();
    data[":address"]        = m_address->text().trimmed();
    data[":city"]           = m_city->text().trimmed();
    data[":gst_number"]     = m_gstNumber->text().trimmed();
    data[":billing_rate"]   = m_billingRate->value();
    data[":contract_start"] = m_contractStart->date().toString("yyyy-MM-dd");
    data[":contract_end"]   = m_contractEnd->date().toString("yyyy-MM-dd");
    data[":agreement_path"] = m_agreementPath->text().trimmed();
    data[":invoice_terms"]  = m_invoiceTerms->toPlainText();
    data[":status"]         = m_status->currentText();
    data[":notes"]          = m_notes->toPlainText();

    auto& db = DatabaseManager::instance();
    bool success;

    if (m_editMode) {
        data[":id"] = m_clientId;
        success = db.executeNonQuery(
            "UPDATE Clients SET "
            "client_code = :client_code, company_name = :company_name, "
            "contact_person = :contact_person, mobile = :mobile, "
            "email = :email, address = :address, city = :city, "
            "gst_number = :gst_number, billing_rate = :billing_rate, "
            "contract_start = :contract_start, contract_end = :contract_end, "
            "agreement_path = :agreement_path, invoice_terms = :invoice_terms, "
            "status = :status, notes = :notes, "
            "updated_at = datetime('now','localtime') "
            "WHERE id = :id",
            data
        );
    } else {
        success = db.executeNonQuery(
            "INSERT INTO Clients ("
            "client_code, company_name, contact_person, mobile, "
            "email, address, city, gst_number, billing_rate, "
            "contract_start, contract_end, agreement_path, "
            "invoice_terms, status, notes"
            ") VALUES ("
            ":client_code, :company_name, :contact_person, :mobile, "
            ":email, :address, :city, :gst_number, :billing_rate, "
            ":contract_start, :contract_end, :agreement_path, "
            ":invoice_terms, :status, :notes"
            ")",
            data
        );
    }

    if (success) {
        accept();
    } else {
        m_errorLabel->setText("Failed to save client. Client code may already exist.");
        m_errorLabel->show();
    }
}
