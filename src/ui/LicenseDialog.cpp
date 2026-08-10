#include "LicenseDialog.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QSysInfo>
#include <QDateTime>
#include <QApplication>

static const QString SECRET_KEY = "SGMS2025SECURITY@KEY";

static QString getMachineId()
{
    QString info = QSysInfo::machineHostName()
        + QSysInfo::productType()
        + QSysInfo::machineUniqueId()
        + QSysInfo::currentCpuArchitecture()
        + QSysInfo::kernelType()
        + QSysInfo::prettyProductName();

    return QCryptographicHash::hash(info.toUtf8(), QCryptographicHash::Sha256).toHex().left(16).toUpper();
}

static QString makeChecksum(const QString& keyPart, const QString& machineId)
{
    QString combined = keyPart.toUpper() + machineId + SECRET_KEY;
    QByteArray hash = QCryptographicHash::hash(combined.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex().left(8).toUpper();
}

static bool checkFormat(const QString& license)
{
    if (license.length() != 19) return false;
    if (license[4] != '-' || license[9] != '-' || license[14] != '-') return false;
    for (int i = 0; i < 19; ++i) {
        if (i == 4 || i == 9 || i == 14) continue;
        QChar c = license[i].toUpper();
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z'))) return false;
    }
    return true;
}

LicenseDialog::LicenseDialog(QWidget* parent) : QDialog(parent)
{
    buildUI();
}

void LicenseDialog::buildUI()
{
    setWindowTitle("Software License Activation");
    setFixedSize(520, 420);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* headerLabel = new QLabel("SGMS License Activation");
    headerLabel->setStyleSheet("color: #D4B44C; font-size: 20px; font-weight: 700;");
    headerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(headerLabel);

    auto* descLabel = new QLabel("Enter your license key to activate the Security Guard Management System.");
    descLabel->setStyleSheet("color: #8B95A5; font-size: 13px;");
    descLabel->setWordWrap(true);
    descLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(descLabel);
    mainLayout->addSpacing(8);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* formGroup = new QGroupBox("License Key");
    formGroup->setStyleSheet("QGroupBox { color: #8B95A5; font-weight: 600; border: 1px solid #2A3545; border-radius: 8px; padding-top: 16px; margin-top: 8px; } QGroupBox::title { padding: 0 12px; }");
    auto* formLayout = new QVBoxLayout(formGroup);
    formLayout->setSpacing(12);

    m_licenseEdit = new QLineEdit;
    m_licenseEdit->setPlaceholderText("XXXX-XXXX-XXXX-XXXX");
    m_licenseEdit->setAlignment(Qt::AlignCenter);
    m_licenseEdit->setMaxLength(19);
    m_licenseEdit->setStyleSheet(
        "QLineEdit { font-size: 20px; font-weight: bold; letter-spacing: 4px; "
        "padding: 12px; background-color: #1E2530; border: 2px solid #3D4654; "
        "border-radius: 8px; color: #D4B44C; } "
        "QLineEdit:focus { border-color: #D4B44C; }");
    formLayout->addWidget(m_licenseEdit);

    auto* machineIdLabel = new QLabel(QString("Machine ID: %1").arg(getMachineId()));
    machineIdLabel->setStyleSheet("color: #6B7585; font-size: 11px; font-family: monospace;");
    machineIdLabel->setAlignment(Qt::AlignCenter);
    formLayout->addWidget(machineIdLabel);

    mainLayout->addWidget(formGroup);

    m_statusLabel = new QLabel;
    m_statusLabel->setStyleSheet("color: #60A5FA; font-size: 12px;");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addStretch();

    auto* btnRow = new QHBoxLayout;
    btnRow->setSpacing(12);
    auto* exitBtn = new QPushButton("Exit");
    exitBtn->setFixedSize(120, 44);
    exitBtn->setCursor(Qt::PointingHandCursor);
    exitBtn->setStyleSheet(
        "QPushButton { background-color: #2A1A1A; color: #E85454; border: 1px solid #4A2020; "
        "border-radius: 6px; padding: 8px 20px; font-weight: 600; font-size: 14px; } "
        "QPushButton:hover { background-color: #3A2020; }");
    connect(exitBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(exitBtn);
    btnRow->addStretch();
    auto* activateBtn = new QPushButton("Activate License");
    activateBtn->setFixedSize(180, 44);
    activateBtn->setCursor(Qt::PointingHandCursor);
    activateBtn->setStyleSheet(
        "QPushButton { background-color: #D4B44C; color: #0D1117; border: none; "
        "border-radius: 6px; padding: 8px 20px; font-weight: 700; font-size: 14px; } "
        "QPushButton:hover { background-color: #E8C547; }");
    connect(activateBtn, &QPushButton::clicked, this, &LicenseDialog::activateLicense);
    btnRow->addWidget(activateBtn);
    mainLayout->addLayout(btnRow);

    auto* footerLabel = new QLabel("Contact your administrator to obtain a license key.");
    footerLabel->setStyleSheet("color: #6B7585; font-size: 11px; padding-top: 8px;");
    footerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(footerLabel);
}

void LicenseDialog::activateLicense()
{
    QString license = m_licenseEdit->text().trimmed().toUpper();

    if (license.isEmpty()) {
        m_errorLabel->setText("Please enter a license key.");
        m_errorLabel->show();
        return;
    }
    if (!checkFormat(license)) {
        m_errorLabel->setText("Invalid format. Use: XXXX-XXXX-XXXX-XXXX");
        m_errorLabel->show();
        return;
    }
    m_errorLabel->hide();

    QString keyClean = license;
    keyClean.remove('-');
    QString userChecksum = keyClean.right(8).toUpper();
    QString keyPart = keyClean.left(8).toUpper();
    QString machineId = getMachineId();

    QString machineCheck = makeChecksum(keyPart, machineId);
    QString universalCheck = makeChecksum(keyPart, "UNIVERSAL");
    bool valid = (userChecksum == machineCheck) || (userChecksum == universalCheck);

    if (!valid) {
        m_errorLabel->setText("Invalid license key. Please check and try again.");
        m_errorLabel->show();
        m_licenseEdit->selectAll();
        m_licenseEdit->setFocus();
        return;
    }

    auto& db = DatabaseManager::instance();
    db.executeNonQuery("INSERT OR REPLACE INTO Settings (key, value) VALUES ('license_key', :key)", {{":key", license}});
    db.executeNonQuery("INSERT OR REPLACE INTO Settings (key, value) VALUES ('license_status', 'active')", {{}});
    db.executeNonQuery("INSERT OR REPLACE INTO Settings (key, value) VALUES ('license_activated', :date)", {{":date", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")}});
    db.executeNonQuery("INSERT OR REPLACE INTO Settings (key, value) VALUES ('license_machine', :mid)", {{":mid", machineId}});

    m_statusLabel->setStyleSheet("color: #4ADE80; font-size: 13px; font-weight: 700;");
    m_statusLabel->setText("License activated successfully!");
    QMessageBox::information(this, "License Activated",
        "Your license has been activated successfully.\n\nThe application will now start.");
    accept();
}

bool LicenseDialog::isLicenseValid()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT value FROM Settings WHERE key = 'license_status'");
    if (q.next()) return q.value("value").toString() == "active";
    return false;
}
