#include "SiteDialog.h"
#include "database/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QMessageBox>

SiteDialog::SiteDialog(QWidget* parent, int siteId)
    : QDialog(parent), m_siteId(siteId), m_editMode(siteId > 0)
{
    buildUI();
    setWindowTitle(m_editMode ? "Edit Site" : "Add New Site");
    setMinimumSize(650, 640);
    setModal(true);

    loadClients();
    loadSupervisors();

    if (m_editMode) {
        loadSiteData();
    }
}

void SiteDialog::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel(m_editMode ? "EDIT SITE" : "ADD NEW SITE");
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

    // ---- Site Information ----
    auto* siteGroup = new QGroupBox("Site Information");
    auto* siteForm = new QFormLayout(siteGroup);
    siteForm->setSpacing(10);
    siteForm->setLabelAlignment(Qt::AlignRight);

    m_siteCode = new QLineEdit;
    m_siteCode->setPlaceholderText("e.g., SITE-001");
    siteForm->addRow("Site Code *:", m_siteCode);

    m_siteName = new QLineEdit;
    m_siteName->setPlaceholderText("Site or location name");
    siteForm->addRow("Site Name *:", m_siteName);

    m_address = new QLineEdit;
    m_address->setPlaceholderText("Full address");
    siteForm->addRow("Address:", m_address);

    m_city = new QLineEdit;
    m_city->setPlaceholderText("City");
    siteForm->addRow("City:", m_city);

    m_clientCombo = new QComboBox;
    m_clientCombo->addItem("-- Select Client --", 0);
    siteForm->addRow("Client *:", m_clientCombo);

    m_guardsRequired = new QSpinBox;
    m_guardsRequired->setRange(1, 999);
    m_guardsRequired->setValue(1);
    siteForm->addRow("Guards Required:", m_guardsRequired);

    formLayout->addWidget(siteGroup);

    // ---- Shift Timings ----
    auto* shiftGroup = new QGroupBox("Shift Timings");
    auto* shiftForm = new QFormLayout(shiftGroup);
    shiftForm->setSpacing(10);
    shiftForm->setLabelAlignment(Qt::AlignRight);

    m_shiftMorning = new QLineEdit;
    m_shiftMorning->setPlaceholderText("e.g., 06:00-14:00");
    m_shiftMorning->setText("06:00-14:00");
    shiftForm->addRow("Morning Shift:", m_shiftMorning);

    m_shiftAfternoon = new QLineEdit;
    m_shiftAfternoon->setPlaceholderText("e.g., 14:00-22:00");
    m_shiftAfternoon->setText("14:00-22:00");
    shiftForm->addRow("Afternoon Shift:", m_shiftAfternoon);

    m_shiftNight = new QLineEdit;
    m_shiftNight->setPlaceholderText("e.g., 22:00-06:00");
    m_shiftNight->setText("22:00-06:00");
    shiftForm->addRow("Night Shift:", m_shiftNight);

    formLayout->addWidget(shiftGroup);

    // ---- Supervisor & Status ----
    auto* supervisorGroup = new QGroupBox("Supervisor & Status");
    auto* supervisorForm = new QFormLayout(supervisorGroup);
    supervisorForm->setSpacing(10);
    supervisorForm->setLabelAlignment(Qt::AlignRight);

    m_supervisorCombo = new QComboBox;
    m_supervisorCombo->addItem("-- No Supervisor --", 0);
    supervisorForm->addRow("Supervisor:", m_supervisorCombo);

    m_status = new QComboBox;
    m_status->addItems({"Active", "Inactive", "Under Construction", "Contract Ended"});
    supervisorForm->addRow("Status:", m_status);

    formLayout->addWidget(supervisorGroup);

    // ---- Instructions ----
    auto* instrGroup = new QGroupBox("Site Instructions");
    auto* instrLayout = new QVBoxLayout(instrGroup);
    m_siteInstructions = new QTextEdit;
    m_siteInstructions->setPlaceholderText(
        "Special instructions for this site:\n"
        "- Gate checking procedures\n"
        "- Emergency contacts\n"
        "- Restricted areas\n"
        "- Uniform requirements\n"
        "- Reporting schedule"
    );
    m_siteInstructions->setMaximumHeight(120);
    instrLayout->addWidget(m_siteInstructions);
    formLayout->addWidget(instrGroup);

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

    auto* saveBtn = new QPushButton(m_editMode ? "Update Site" : "Save Site");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(140, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &SiteDialog::saveSite);
    btnRow->addWidget(saveBtn);

    mainLayout->addLayout(btnRow);
}

void SiteDialog::loadClients()
{
    auto& db = DatabaseManager::instance();
    QSqlQuery query = db.execute(
        "SELECT id, company_name FROM Clients WHERE status = 'Active' ORDER BY company_name"
    );
    while (query.next()) {
        m_clientCombo->addItem(
            query.value("company_name").toString(),
            query.value("id").toInt()
        );
    }
}

void SiteDialog::loadSupervisors()
{
    auto& db = DatabaseManager::instance();
    QSqlQuery query = db.execute(
        "SELECT id, full_name, guard_code FROM Guards "
        "WHERE status = 'Active' ORDER BY full_name"
    );
    while (query.next()) {
        m_supervisorCombo->addItem(
            QString("%1 (%2)")
                .arg(query.value("full_name").toString())
                .arg(query.value("guard_code").toString()),
            query.value("id").toInt()
        );
    }
}

void SiteDialog::loadSiteData()
{
    auto& db = DatabaseManager::instance();
    QSqlQuery query = db.execute(
        "SELECT * FROM Sites WHERE id = :id", {{":id", m_siteId}}
    );
    if (!query.next()) return;

    m_siteCode->setText(query.value("site_code").toString());
    m_siteName->setText(query.value("site_name").toString());
    m_address->setText(query.value("address").toString());
    m_city->setText(query.value("city").toString());
    m_shiftMorning->setText(query.value("shift_morning").toString());
    m_shiftAfternoon->setText(query.value("shift_afternoon").toString());
    m_shiftNight->setText(query.value("shift_night").toString());
    m_guardsRequired->setValue(query.value("guards_required").toInt());
    m_siteInstructions->setPlainText(query.value("site_instructions").toString());
    m_status->setCurrentText(query.value("status").toString());

    // Set client combo
    int clientId = query.value("client_id").toInt();
    for (int i = 0; i < m_clientCombo->count(); ++i) {
        if (m_clientCombo->itemData(i).toInt() == clientId) {
            m_clientCombo->setCurrentIndex(i);
            break;
        }
    }

    // Set supervisor combo
    int supervisorId = query.value("supervisor_id").toInt();
    for (int i = 0; i < m_supervisorCombo->count(); ++i) {
        if (m_supervisorCombo->itemData(i).toInt() == supervisorId) {
            m_supervisorCombo->setCurrentIndex(i);
            break;
        }
    }
}

bool SiteDialog::validate()
{
    if (m_siteCode->text().trimmed().isEmpty()) {
        m_errorLabel->setText("Site code is required.");
        m_errorLabel->show();
        m_siteCode->setFocus();
        return false;
    }
    if (m_siteName->text().trimmed().isEmpty()) {
        m_errorLabel->setText("Site name is required.");
        m_errorLabel->show();
        m_siteName->setFocus();
        return false;
    }
    if (m_clientCombo->currentData().toInt() == 0) {
        m_errorLabel->setText("Please select a client.");
        m_errorLabel->show();
        m_clientCombo->setFocus();
        return false;
    }

    m_errorLabel->hide();
    return true;
}

void SiteDialog::saveSite()
{
    if (!validate()) return;

    QVariantMap data;
    data[":site_code"]         = m_siteCode->text().trimmed();
    data[":site_name"]         = m_siteName->text().trimmed();
    data[":address"]           = m_address->text().trimmed();
    data[":city"]              = m_city->text().trimmed();
    data[":client_id"]         = m_clientCombo->currentData().toInt();
    data[":shift_morning"]     = m_shiftMorning->text().trimmed();
    data[":shift_afternoon"]   = m_shiftAfternoon->text().trimmed();
    data[":shift_night"]       = m_shiftNight->text().trimmed();

    int supId = m_supervisorCombo->currentData().toInt();
    data[":supervisor_id"]     = supId > 0 ? supId : QVariant();

    data[":guards_required"]   = m_guardsRequired->value();
    data[":site_instructions"] = m_siteInstructions->toPlainText();
    data[":status"]            = m_status->currentText();

    auto& db = DatabaseManager::instance();
    bool success;

    if (m_editMode) {
        data[":id"] = m_siteId;
        success = db.executeNonQuery(
            "UPDATE Sites SET "
            "site_code = :site_code, site_name = :site_name, "
            "address = :address, city = :city, client_id = :client_id, "
            "shift_morning = :shift_morning, shift_afternoon = :shift_afternoon, "
            "shift_night = :shift_night, supervisor_id = :supervisor_id, "
            "guards_required = :guards_required, site_instructions = :site_instructions, "
            "status = :status, updated_at = datetime('now','localtime') "
            "WHERE id = :id",
            data
        );
    } else {
        success = db.executeNonQuery(
            "INSERT INTO Sites ("
            "site_code, site_name, address, city, client_id, "
            "shift_morning, shift_afternoon, shift_night, "
            "supervisor_id, guards_required, site_instructions, status"
            ") VALUES ("
            ":site_code, :site_name, :address, :city, :client_id, "
            ":shift_morning, :shift_afternoon, :shift_night, "
            ":supervisor_id, :guards_required, :site_instructions, :status"
            ")",
            data
        );
    }

    if (success) {
        accept();
    } else {
        m_errorLabel->setText("Failed to save site. Site code may already exist.");
        m_errorLabel->show();
    }
}
