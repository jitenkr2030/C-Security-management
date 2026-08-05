#include "InvoiceDialog.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDate>
#include <QScrollArea>

InvoiceDialog::InvoiceDialog(QWidget* parent, int invoiceId)
    : QDialog(parent), m_invoiceId(invoiceId), m_editMode(invoiceId > 0)
{
    buildUI();
    loadClientCombo();
    loadSiteCombo();
    if (m_editMode) loadInvoiceData();
}

void InvoiceDialog::buildUI()
{
    setWindowTitle(m_editMode ? "Edit Invoice" : "Generate Invoice");
    setMinimumSize(560, 620);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(12);

    auto* titleLabel = new QLabel(m_editMode ? "EDIT INVOICE" : "GENERATE CLIENT INVOICE");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto* formWidget = new QWidget;
    auto* form = new QFormLayout(formWidget);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_codeEdit = new QLineEdit;
    m_codeEdit->setPlaceholderText("e.g. INV-2025-001");
    form->addRow("Invoice Code *:", m_codeEdit);

    m_clientCombo = new QComboBox;
    m_clientCombo->addItem("-- Select Client --", 0);
    form->addRow("Client *:", m_clientCombo);

    m_monthCombo = new QComboBox;
    m_monthCombo->addItems({"January", "February", "March", "April", "May", "June",
                            "July", "August", "September", "October", "November", "December"});
    m_monthCombo->setCurrentIndex(QDate::currentDate().month() - 1);
    form->addRow("Invoice Month:", m_monthCombo);

    m_yearSpin = new QSpinBox;
    m_yearSpin->setRange(2020, 2035);
    m_yearSpin->setValue(QDate::currentDate().year());
    form->addRow("Year:", m_yearSpin);

    m_siteCombo = new QComboBox;
    m_siteCombo->addItem("-- All Sites --", 0);
    connect(m_siteCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InvoiceDialog::autoFillFromSite);
    form->addRow("Site:", m_siteCombo);

    m_guardsDeployed = new QSpinBox;
    m_guardsDeployed->setRange(0, 999);
    connect(m_guardsDeployed, QOverload<int>::of(&QSpinBox::valueChanged), this, &InvoiceDialog::calculateTotal);
    form->addRow("Guards Deployed:", m_guardsDeployed);

    m_workingDays = new QSpinBox;
    m_workingDays->setRange(0, 31);
    m_workingDays->setValue(26);
    connect(m_workingDays, QOverload<int>::of(&QSpinBox::valueChanged), this, &InvoiceDialog::calculateTotal);
    form->addRow("Working Days:", m_workingDays);

    m_perGuardRate = new QDoubleSpinBox;
    m_perGuardRate->setRange(0, 100000);
    m_perGuardRate->setDecimals(0);
    m_perGuardRate->setPrefix("Rs. ");
    m_perGuardRate->setSingleStep(500);
    connect(m_perGuardRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &InvoiceDialog::calculateTotal);
    form->addRow("Per Guard/Day Rate:", m_perGuardRate);

    m_totalGuardCharges = new QDoubleSpinBox;
    m_totalGuardCharges->setRange(0, 10000000);
    m_totalGuardCharges->setDecimals(0);
    m_totalGuardCharges->setPrefix("Rs. ");
    m_totalGuardCharges->setReadOnly(true);
    m_totalGuardCharges->setButtonSymbols(QAbstractSpinBox::NoButtons);
    form->addRow("Guard Charges:", m_totalGuardCharges);

    m_equipmentCharges = new QDoubleSpinBox;
    m_equipmentCharges->setRange(0, 1000000);
    m_equipmentCharges->setDecimals(0);
    m_equipmentCharges->setPrefix("Rs. ");
    m_equipmentCharges->setSingleStep(1000);
    connect(m_equipmentCharges, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &InvoiceDialog::calculateTotal);
    form->addRow("Equipment Charges:", m_equipmentCharges);

    m_otherCharges = new QDoubleSpinBox;
    m_otherCharges->setRange(0, 1000000);
    m_otherCharges->setDecimals(0);
    m_otherCharges->setPrefix("Rs. ");
    m_otherCharges->setSingleStep(1000);
    connect(m_otherCharges, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &InvoiceDialog::calculateTotal);
    form->addRow("Other Charges:", m_otherCharges);

    m_subtotal = new QDoubleSpinBox;
    m_subtotal->setRange(0, 10000000);
    m_subtotal->setDecimals(0);
    m_subtotal->setPrefix("Rs. ");
    m_subtotal->setReadOnly(true);
    m_subtotal->setButtonSymbols(QAbstractSpinBox::NoButtons);
    form->addRow("Subtotal:", m_subtotal);

    m_gstRate = new QDoubleSpinBox;
    m_gstRate->setRange(0, 50);
    m_gstRate->setDecimals(1);
    m_gstRate->setSuffix(" %");
    m_gstRate->setValue(18);
    connect(m_gstRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &InvoiceDialog::calculateTotal);
    form->addRow("GST Rate:", m_gstRate);

    m_gstAmount = new QDoubleSpinBox;
    m_gstAmount->setRange(0, 10000000);
    m_gstAmount->setDecimals(0);
    m_gstAmount->setPrefix("Rs. ");
    m_gstAmount->setReadOnly(true);
    m_gstAmount->setButtonSymbols(QAbstractSpinBox::NoButtons);
    form->addRow("GST Amount:", m_gstAmount);

    m_totalAmount = new QDoubleSpinBox;
    m_totalAmount->setRange(0, 100000000);
    m_totalAmount->setDecimals(0);
    m_totalAmount->setPrefix("Rs. ");
    m_totalAmount->setReadOnly(true);
    m_totalAmount->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_totalAmount->setStyleSheet("QDoubleSpinBox { font-weight: bold; color: #D4B44C; }");
    form->addRow("TOTAL AMOUNT:", m_totalAmount);

    m_invoiceDate = new QDateEdit;
    m_invoiceDate->setCalendarPopup(true);
    m_invoiceDate->setDisplayFormat("yyyy-MM-dd");
    m_invoiceDate->setDate(QDate::currentDate());
    form->addRow("Invoice Date:", m_invoiceDate);

    m_dueDate = new QDateEdit;
    m_dueDate->setCalendarPopup(true);
    m_dueDate->setDisplayFormat("yyyy-MM-dd");
    m_dueDate->setDate(QDate::currentDate().addDays(30));
    form->addRow("Due Date:", m_dueDate);

    m_statusCombo = new QComboBox;
    m_statusCombo->addItems({"Draft", "Sent", "Paid", "Overdue", "Cancelled"});
    form->addRow("Status:", m_statusCombo);

    m_notes = new QTextEdit;
    m_notes->setPlaceholderText("Additional notes...");
    m_notes->setMaximumHeight(50);
    form->addRow("Notes:", m_notes);

    m_calcLabel = new QLabel;
    m_calcLabel->setStyleSheet("color: #60A5FA; font-size: 11px; font-style: italic;");
    form->addRow("", m_calcLabel);

    scrollArea->setWidget(formWidget);
    mainLayout->addWidget(scrollArea, 1);

    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("SecondaryButton");
    cancelBtn->setFixedSize(100, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);
    auto* saveBtn = new QPushButton(m_editMode ? "Update" : "Generate Invoice");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(160, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &InvoiceDialog::saveInvoice);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void InvoiceDialog::loadClientCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT id, client_name FROM Clients WHERE status = 'Active' ORDER BY client_name");
    while (q.next()) m_clientCombo->addItem(q.value("client_name").toString(), q.value("id").toInt());
}

void InvoiceDialog::loadSiteCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT id, site_name, guards_required, monthly_charge FROM Sites WHERE status = 'Active' ORDER BY site_name");
    while (q.next()) {
        int sid = q.value("id").toInt();
        QString name = q.value("site_name").toString();
        int guards = q.value("guards_required").toInt();
        m_siteCombo->addItem(QString("%1 (%2 guards)").arg(name).arg(guards), sid);
    }
}

void InvoiceDialog::autoFillFromSite()
{
    int siteId = m_siteCombo->currentData().toInt();
    if (siteId == 0) return;

    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT guards_required, monthly_charge FROM Sites WHERE id = :id", {{":id", siteId}});
    if (q.next()) {
        int guards = q.value("guards_required").toInt();
        double charge = q.value("monthly_charge").toDouble();
        m_guardsDeployed->setValue(guards);
        if (charge > 0 && guards > 0) {
            m_perGuardRate->setValue(charge / guards / 26.0);
        }
        calculateTotal();
    }
}

void InvoiceDialog::calculateTotal()
{
    double guardCharges = m_guardsDeployed->value() * m_workingDays->value() * m_perGuardRate->value();
    m_totalGuardCharges->setValue(guardCharges);

    double subtotal = guardCharges + m_equipmentCharges->value() + m_otherCharges->value();
    m_subtotal->setValue(subtotal);

    double gst = subtotal * m_gstRate->value() / 100.0;
    m_gstAmount->setValue(gst);

    m_totalAmount->setValue(subtotal + gst);

    m_calcLabel->setText(QString("%1 guards x %2 days x Rs.%3 = Rs.%4 | + GST %5% = Rs.%6")
        .arg(m_guardsDeployed->value()).arg(m_workingDays->value())
        .arg(m_perGuardRate->value(), 0, 'f', 0).arg(guardCharges, 0, 'f', 0)
        .arg(m_gstRate->value(), 0, 'f', 1).arg(gst, 0, 'f', 0));
}

void InvoiceDialog::loadInvoiceData()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT * FROM Invoices WHERE id = :id", {{":id", m_invoiceId}});
    if (!q.next()) return;

    m_codeEdit->setText(q.value("invoice_code").toString());
    int clientId = q.value("client_id").toInt();
    for (int i = 0; i < m_clientCombo->count(); ++i)
        if (m_clientCombo->itemData(i).toInt() == clientId) { m_clientCombo->setCurrentIndex(i); break; }
    m_monthCombo->setCurrentIndex(q.value("invoice_month").toInt() - 1);
    m_yearSpin->setValue(q.value("invoice_year").toInt());
    int siteId = q.value("site_id").toInt();
    for (int i = 0; i < m_siteCombo->count(); ++i)
        if (m_siteCombo->itemData(i).toInt() == siteId) { m_siteCombo->setCurrentIndex(i); break; }
    m_guardsDeployed->setValue(q.value("guards_deployed").toInt());
    m_workingDays->setValue(q.value("working_days").toInt());
    m_perGuardRate->setValue(q.value("per_guard_rate").toDouble());
    m_totalGuardCharges->setValue(q.value("total_guard_charges").toDouble());
    m_equipmentCharges->setValue(q.value("equipment_charges").toDouble());
    m_otherCharges->setValue(q.value("other_charges").toDouble());
    m_subtotal->setValue(q.value("subtotal").toDouble());
    m_gstRate->setValue(q.value("gst_rate").toDouble());
    m_gstAmount->setValue(q.value("gst_amount").toDouble());
    m_totalAmount->setValue(q.value("total_amount").toDouble());
    m_invoiceDate->setDate(QDate::fromString(q.value("invoice_date").toString(), "yyyy-MM-dd"));
    m_dueDate->setDate(QDate::fromString(q.value("due_date").toString(), "yyyy-MM-dd"));
    m_statusCombo->setCurrentText(q.value("status").toString());
    m_notes->setPlainText(q.value("notes").toString());
}

void InvoiceDialog::saveInvoice()
{
    if (m_codeEdit->text().trimmed().isEmpty()) { m_errorLabel->setText("Invoice code is required."); m_errorLabel->show(); return; }
    if (m_clientCombo->currentData().toInt() == 0) { m_errorLabel->setText("Please select a client."); m_errorLabel->show(); return; }
    m_errorLabel->hide();

    auto& db = DatabaseManager::instance();
    QVariantMap data;
    data[":code"] = m_codeEdit->text().trimmed().toUpper();
    data[":cid"] = m_clientCombo->currentData();
    data[":month"] = m_monthCombo->currentIndex() + 1;
    data[":year"] = m_yearSpin->value();
    data[":sid"] = m_siteCombo->currentData().toInt() > 0 ? m_siteCombo->currentData() : QVariant();
    data[":guards"] = m_guardsDeployed->value();
    data[":days"] = m_workingDays->value();
    data[":rate"] = m_perGuardRate->value();
    data[":guardCharges"] = m_totalGuardCharges->value();
    data[":equipCharges"] = m_equipmentCharges->value();
    data[":otherCharges"] = m_otherCharges->value();
    data[":subtotal"] = m_subtotal->value();
    data[":gstRate"] = m_gstRate->value();
    data[":gstAmt"] = m_gstAmount->value();
    data[":total"] = m_totalAmount->value();
    data[":status"] = m_statusCombo->currentText();
    data[":invDate"] = m_invoiceDate->date().toString("yyyy-MM-dd");
    data[":dueDate"] = m_dueDate->date().toString("yyyy-MM-dd");
    data[":notes"] = m_notes->toPlainText().trimmed();

    bool ok;
    if (m_editMode) {
        data[":id"] = m_invoiceId;
        ok = db.executeNonQuery("UPDATE Invoices SET invoice_code=:code, client_id=:cid, invoice_month=:month, invoice_year=:year, site_id=:sid, guards_deployed=:guards, working_days=:days, per_guard_rate=:rate, total_guard_charges=:guardCharges, equipment_charges=:equipCharges, other_charges=:otherCharges, subtotal=:subtotal, gst_rate=:gstRate, gst_amount=:gstAmt, total_amount=:total, status=:status, invoice_date=:invDate, due_date=:dueDate, notes=:notes WHERE id=:id", data);
    } else {
        ok = db.executeNonQuery("INSERT INTO Invoices (invoice_code, client_id, invoice_month, invoice_year, site_id, guards_deployed, working_days, per_guard_rate, total_guard_charges, equipment_charges, other_charges, subtotal, gst_rate, gst_amount, total_amount, status, invoice_date, due_date, notes) VALUES (:code, :cid, :month, :year, :sid, :guards, :days, :rate, :guardCharges, :equipCharges, :otherCharges, :subtotal, :gstRate, :gstAmt, :total, :status, :invDate, :dueDate, :notes)", data);
    }

    if (ok) accept();
    else { m_errorLabel->setText("Failed to save invoice. Code may already exist."); m_errorLabel->show(); }
}
