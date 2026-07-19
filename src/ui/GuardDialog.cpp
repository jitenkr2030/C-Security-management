#include "GuardDialog.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QScrollArea>
#include <QMessageBox>
#include <QFrame>
#include <QDateTime>
GuardDialog::GuardDialog(QWidget* parent, int guardId)
    : QDialog(parent), m_guardId(guardId), m_editMode(guardId > 0)
{
    buildUI();
    setWindowTitle(m_editMode ? "Edit Guard" : "Add New Guard");
    setMinimumSize(720, 680);
    setModal(true);
    loadClientCombo();
    loadSiteCombo();
    if (m_editMode) { loadGuardData(); }
}
void GuardDialog::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);
    auto* titleLabel = new QLabel(m_editMode ? "EDIT GUARD" : "ADD NEW GUARD");
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
    // Personal
    auto* pg = new QGroupBox("Personal Information");
    auto* pf = new QFormLayout(pg);
    pf->setSpacing(10); pf->setLabelAlignment(Qt::AlignRight);
    m_guardCode = new QLineEdit; m_guardCode->setPlaceholderText("GRD-001");
    pf->addRow("Guard Code *:", m_guardCode);
    m_fullName = new QLineEdit; m_fullName->setPlaceholderText("Full name");
    pf->addRow("Full Name *:", m_fullName);
    m_fatherName = new QLineEdit; m_fatherName->setPlaceholderText("Father name");
    pf->addRow("Father Name:", m_fatherName);
    m_dob = new QDateEdit; m_dob->setCalendarPopup(true); m_dob->setDisplayFormat("yyyy-MM-dd"); m_dob->setDate(QDate(1990,1,1));
    pf->addRow("Date of Birth:", m_dob);
    m_gender = new QComboBox; m_gender->addItems({"Male","Female","Other"});
    pf->addRow("Gender:", m_gender);
    m_mobile1 = new QLineEdit; m_mobile1->setPlaceholderText("Primary mobile");
    pf->addRow("Mobile *:", m_mobile1);
    m_mobile2 = new QLineEdit; m_mobile2->setPlaceholderText("Secondary mobile");
    pf->addRow("Alt. Mobile:", m_mobile2);
    m_email = new QLineEdit; m_email->setPlaceholderText("Email");
    pf->addRow("Email:", m_email);
    formLayout->addWidget(pg);
    // Address
    auto* ag = new QGroupBox("Address");
    auto* af = new QFormLayout(ag);
    af->setSpacing(10); af->setLabelAlignment(Qt::AlignRight);
    m_address = new QLineEdit; m_address->setPlaceholderText("Street address");
    af->addRow("Address:", m_address);
    auto* csRow = new QHBoxLayout;
    m_city = new QLineEdit; m_city->setPlaceholderText("City");
    m_state = new QLineEdit; m_state->setPlaceholderText("State");
    csRow->addWidget(m_city); csRow->addWidget(m_state);
    af->addRow("City / State:", csRow);
    m_pincode = new QLineEdit; m_pincode->setPlaceholderText("PIN code");
    af->addRow("Pincode:", m_pincode);
    formLayout->addWidget(ag);
    // Identity
    auto* ig = new QGroupBox("Identity Documents");
    auto* inf = new QFormLayout(ig);
    inf->setSpacing(10); inf->setLabelAlignment(Qt::AlignRight);
    m_aadhaar = new QLineEdit; m_aadhaar->setPlaceholderText("12-digit"); m_aadhaar->setMaxLength(12);
    inf->addRow("Aadhaar:", m_aadhaar);
    m_pan = new QLineEdit; m_pan->setPlaceholderText("PAN"); m_pan->setMaxLength(10);
    inf->addRow("PAN:", m_pan);
    m_policeVerified = new QCheckBox("Police verification completed");
    inf->addRow("Verified:", m_policeVerified);
    formLayout->addWidget(ig);
    // Employment
    auto* eg = new QGroupBox("Employment");
    auto* ef = new QFormLayout(eg);
    ef->setSpacing(10); ef->setLabelAlignment(Qt::AlignRight);
    m_joiningDate = new QDateEdit; m_joiningDate->setCalendarPopup(true); m_joiningDate->setDisplayFormat("yyyy-MM-dd"); m_joiningDate->setDate(QDate::currentDate());
    ef->addRow("Joining Date *:", m_joiningDate);
    m_status = new QComboBox; m_status->addItems({"Active","Inactive","On Leave","Terminated"});
    ef->addRow("Status:", m_status);
    formLayout->addWidget(eg);
    // Site Assignment
    auto* sg = new QGroupBox("Site Assignment");
    auto* sf = new QFormLayout(sg);
    sf->setSpacing(10); sf->setLabelAlignment(Qt::AlignRight);
    m_clientCombo = new QComboBox; m_clientCombo->addItem("-- Select Client --", 0);
    sf->addRow("Client:", m_clientCombo);
    m_siteCombo = new QComboBox; m_siteCombo->addItem("-- No Assignment --", 0);
    sf->addRow("Site:", m_siteCombo);
    formLayout->addWidget(sg);
    // Bank
    auto* bg = new QGroupBox("Bank Details");
    auto* bf = new QFormLayout(bg);
    bf->setSpacing(10); bf->setLabelAlignment(Qt::AlignRight);
    m_bankName = new QLineEdit; m_bankName->setPlaceholderText("Bank name");
    bf->addRow("Bank:", m_bankName);
    m_bankAccount = new QLineEdit; m_bankAccount->setPlaceholderText("Account number");
    bf->addRow("Account:", m_bankAccount);
    m_bankIfsc = new QLineEdit; m_bankIfsc->setPlaceholderText("IFSC");
    bf->addRow("IFSC:", m_bankIfsc);
    m_uan = new QLineEdit; m_uan->setPlaceholderText("UAN");
    bf->addRow("UAN:", m_uan);
    m_esic = new QLineEdit; m_esic->setPlaceholderText("ESIC");
    bf->addRow("ESIC:", m_esic);
    m_pf = new QLineEdit; m_pf->setPlaceholderText("PF");
    bf->addRow("PF:", m_pf);
    formLayout->addWidget(bg);
    // Salary
    auto* salg = new QGroupBox("Salary (Monthly)");
    auto* salf = new QFormLayout(salg);
    salf->setSpacing(10); salf->setLabelAlignment(Qt::AlignRight);
    m_basicSalary = new QDoubleSpinBox; m_basicSalary->setRange(0,999999); m_basicSalary->setPrefix("Rs. "); m_basicSalary->setDecimals(0);
    salf->addRow("Basic:", m_basicSalary);
    m_hra = new QDoubleSpinBox; m_hra->setRange(0,999999); m_hra->setPrefix("Rs. "); m_hra->setDecimals(0);
    salf->addRow("HRA:", m_hra);
    m_conveyance = new QDoubleSpinBox; m_conveyance->setRange(0,999999); m_conveyance->setPrefix("Rs. "); m_conveyance->setDecimals(0);
    salf->addRow("Conveyance:", m_conveyance);
    m_medical = new QDoubleSpinBox; m_medical->setRange(0,999999); m_medical->setPrefix("Rs. "); m_medical->setDecimals(0);
    salf->addRow("Medical:", m_medical);
    m_special = new QDoubleSpinBox; m_special->setRange(0,999999); m_special->setPrefix("Rs. "); m_special->setDecimals(0);
    salf->addRow("Special:", m_special);
    formLayout->addWidget(salg);
    // Notes
    auto* ng = new QGroupBox("Notes");
    auto* nl = new QVBoxLayout(ng);
    m_notes = new QTextEdit; m_notes->setPlaceholderText("Notes..."); m_notes->setMaximumHeight(100);
    nl->addWidget(m_notes);
    formLayout->addWidget(ng);
    scroll->setWidget(formContent);
    mainLayout->addWidget(scroll, 1);
    // Buttons
    auto* btnRow = new QHBoxLayout; btnRow->addStretch();
    auto* cancelBtn = new QPushButton("Cancel"); cancelBtn->setObjectName("SecondaryButton"); cancelBtn->setFixedSize(100,40); cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);
    auto* saveBtn = new QPushButton(m_editMode ? "Update" : "Save Guard"); saveBtn->setObjectName("PrimaryButton"); saveBtn->setFixedSize(140,40); saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &GuardDialog::saveGuard);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}
void GuardDialog::loadClientCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT id, company_name FROM Clients WHERE status='Active' ORDER BY company_name");
    while (q.next()) { m_clientCombo->addItem(q.value("company_name").toString(), q.value("id").toInt()); }
}
void GuardDialog::loadSiteCombo()
{
    auto& db = DatabaseManager::instance();
    m_siteCombo->clear(); m_siteCombo->addItem("-- No Assignment --", 0);
    auto q = db.execute("SELECT s.id, s.site_name, c.company_name FROM Sites s LEFT JOIN Clients c ON s.client_id=c.id WHERE s.status='Active' ORDER BY s.site_name");
    while (q.next()) { m_siteCombo->addItem(q.value("site_name").toString() + " (" + q.value("company_name").toString() + ")", q.value("id").toInt()); }
}
void GuardDialog::loadGuardData()
{
    auto d = DatabaseManager::instance().getGuardById(m_guardId);
    if (d.isEmpty()) return;
    m_guardCode->setText(d["guard_code"].toString());
    m_fullName->setText(d["full_name"].toString());
    m_fatherName->setText(d["father_name"].toString());
    m_dob->setDate(QDate::fromString(d["date_of_birth"].toString(),"yyyy-MM-dd"));
    m_gender->setCurrentText(d["gender"].toString());
    m_mobile1->setText(d["mobile_primary"].toString());
    m_mobile2->setText(d["mobile_secondary"].toString());
    m_email->setText(d["email"].toString());
    m_address->setText(d["address_line1"].toString());
    m_city->setText(d["city"].toString());
    m_state->setText(d["state"].toString());
    m_pincode->setText(d["pincode"].toString());
    m_aadhaar->setText(d["aadhaar_number"].toString());
    m_pan->setText(d["pan_number"].toString());
    m_policeVerified->setChecked(d["police_verified"].toBool());
    m_joiningDate->setDate(QDate::fromString(d["joining_date"].toString(),"yyyy-MM-dd"));
    m_status->setCurrentText(d["status"].toString());
    m_bankName->setText(d["bank_name"].toString());
    m_bankAccount->setText(d["bank_account"].toString());
    m_bankIfsc->setText(d["bank_ifsc"].toString());
    m_uan->setText(d["uan_number"].toString());
    m_esic->setText(d["esic_number"].toString());
    m_pf->setText(d["pf_number"].toString());
    m_basicSalary->setValue(d["basic_salary"].toDouble());
    m_hra->setValue(d["hra"].toDouble());
    m_conveyance->setValue(d["conveyance"].toDouble());
    m_medical->setValue(d["medical_allowance"].toDouble());
    m_special->setValue(d["special_allowance"].toDouble());
    m_notes->setPlainText(d["notes"].toString());
    int siteId = d["site_id"].toInt();
    for (int i=0;i<m_siteCombo->count();++i) { if (m_siteCombo->itemData(i).toInt()==siteId) { m_siteCombo->setCurrentIndex(i); break; } }
    int clientId = d["client_id"].toInt();
    for (int i=0;i<m_clientCombo->count();++i) { if (m_clientCombo->itemData(i).toInt()==clientId) { m_clientCombo->setCurrentIndex(i); break; } }
}
bool GuardDialog::validate()
{
    if (m_guardCode->text().trimmed().isEmpty()) { m_errorLabel->setText("Guard code required."); m_errorLabel->show(); m_guardCode->setFocus(); return false; }
    if (m_fullName->text().trimmed().isEmpty()) { m_errorLabel->setText("Full name required."); m_errorLabel->show(); m_fullName->setFocus(); return false; }
    if (m_mobile1->text().trimmed().isEmpty()) { m_errorLabel->setText("Mobile required."); m_errorLabel->show(); m_mobile1->setFocus(); return false; }
    m_errorLabel->hide(); return true;
}
void GuardDialog::saveGuard()
{
    if (!validate()) return;
    QVariantMap data;
    data["guard_code"] = m_guardCode->text().trimmed();
    data["full_name"] = m_fullName->text().trimmed();
    data["father_name"] = m_fatherName->text().trimmed();
    data["date_of_birth"] = m_dob->date().toString("yyyy-MM-dd");
    data["gender"] = m_gender->currentText();
    data["mobile_primary"] = m_mobile1->text().trimmed();
    data["mobile_secondary"] = m_mobile2->text().trimmed();
    data["email"] = m_email->text().trimmed();
    data["address_line1"] = m_address->text().trimmed();
    data["city"] = m_city->text().trimmed();
    data["state"] = m_state->text().trimmed();
    data["pincode"] = m_pincode->text().trimmed();
    data["aadhaar_number"] = m_aadhaar->text().trimmed();
    data["pan_number"] = m_pan->text().trimmed();
    data["police_verified"] = m_policeVerified->isChecked() ? 1 : 0;
    data["joining_date"] = m_joiningDate->date().toString("yyyy-MM-dd");
    data["status"] = m_status->currentText();
    data["bank_name"] = m_bankName->text().trimmed();
    data["bank_account"] = m_bankAccount->text().trimmed();
    data["bank_ifsc"] = m_bankIfsc->text().trimmed();
    data["uan_number"] = m_uan->text().trimmed();
    data["esic_number"] = m_esic->text().trimmed();
    data["pf_number"] = m_pf->text().trimmed();
    data["basic_salary"] = m_basicSalary->value();
    data["hra"] = m_hra->value();
    data["conveyance"] = m_conveyance->value();
    data["medical_allowance"] = m_medical->value();
    data["special_allowance"] = m_special->value();
    data["notes"] = m_notes->toPlainText();
    int siteId = m_siteCombo->currentData().toInt();
    data["site_id"] = siteId > 0 ? siteId : QVariant();
    int clientId = m_clientCombo->currentData().toInt();
    if (siteId > 0 && clientId == 0) {
        auto& db = DatabaseManager::instance();
        auto cq = db.execute("SELECT client_id FROM Sites WHERE id=:id", {{":id", siteId}});
        if (cq.next()) clientId = cq.value("client_id").toInt();
    }
    data["client_id"] = clientId > 0 ? clientId : QVariant();
    auto& db = DatabaseManager::instance();
    bool ok;
    if (m_editMode) { ok = db.updateGuard(m_guardId, data); }
    else { ok = db.insertGuard(data); }
    if (ok) { accept(); }
    else { m_errorLabel->setText("Save failed. Code may already exist."); m_errorLabel->show(); }
}
QVariantMap GuardDialog::guardData() const { return QVariantMap(); }
