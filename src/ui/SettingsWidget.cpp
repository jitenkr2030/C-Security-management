#include "SettingsWidget.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QMessageBox>

SettingsWidget::SettingsWidget(QWidget* parent) : QWidget(parent)
{
    buildUI();
    loadSettings();
}

void SettingsWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Settings");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);
    auto* subtitle = new QLabel("Configure company details, salary rules, and leave policies");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    auto* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto* scrollWidget = new QWidget;
    auto* scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setSpacing(16);

    // Company Information
    auto* companyGroup = new QGroupBox("Company Information");
    auto* companyForm = new QFormLayout(companyGroup);
    companyForm->setSpacing(10);
    companyForm->setLabelAlignment(Qt::AlignRight);

    m_companyName = new QLineEdit;
    m_companyName->setPlaceholderText("Company name");
    companyForm->addRow("Company Name:", m_companyName);

    m_companyAddress = new QLineEdit;
    m_companyAddress->setPlaceholderText("Company address");
    companyForm->addRow("Address:", m_companyAddress);

    m_companyPhone = new QLineEdit;
    m_companyPhone->setPlaceholderText("Phone number");
    companyForm->addRow("Phone:", m_companyPhone);

    m_companyEmail = new QLineEdit;
    m_companyEmail->setPlaceholderText("Email address");
    companyForm->addRow("Email:", m_companyEmail);

    scrollLayout->addWidget(companyGroup);

    // Salary Rules
    auto* salaryGroup = new QGroupBox("Salary Rules");
    auto* salaryForm = new QFormLayout(salaryGroup);
    salaryForm->setSpacing(10);
    salaryForm->setLabelAlignment(Qt::AlignRight);

    m_pfRate = new QDoubleSpinBox;
    m_pfRate->setRange(0, 100);
    m_pfRate->setDecimals(2);
    m_pfRate->setSuffix(" %");
    m_pfRate->setValue(12);
    salaryForm->addRow("PF Rate:", m_pfRate);

    m_esicRate = new QDoubleSpinBox;
    m_esicRate->setRange(0, 100);
    m_esicRate->setDecimals(2);
    m_esicRate->setSuffix(" %");
    m_esicRate->setValue(0.75);
    salaryForm->addRow("ESIC Rate:", m_esicRate);

    m_ptDeduction = new QDoubleSpinBox;
    m_ptDeduction->setRange(0, 10000);
    m_ptDeduction->setDecimals(0);
    m_ptDeduction->setPrefix("Rs. ");
    m_ptDeduction->setValue(200);
    salaryForm->addRow("Professional Tax:", m_ptDeduction);

    m_overtimeRate = new QDoubleSpinBox;
    m_overtimeRate->setRange(1, 5);
    m_overtimeRate->setDecimals(1);
    m_overtimeRate->setSuffix("x");
    m_overtimeRate->setValue(1.5);
    salaryForm->addRow("Overtime Rate:", m_overtimeRate);

    m_workingDays = new QSpinBox;
    m_workingDays->setRange(20, 31);
    m_workingDays->setValue(26);
    salaryForm->addRow("Working Days/Month:", m_workingDays);

    scrollLayout->addWidget(salaryGroup);

    // Leave Policy
    auto* leaveGroup = new QGroupBox("Leave Policy (Annual Entitlements)");
    auto* leaveForm = new QFormLayout(leaveGroup);
    leaveForm->setSpacing(10);
    leaveForm->setLabelAlignment(Qt::AlignRight);

    m_casualLeave = new QSpinBox;
    m_casualLeave->setRange(0, 30);
    m_casualLeave->setValue(12);
    leaveForm->addRow("Casual Leave:", m_casualLeave);

    m_sickLeave = new QSpinBox;
    m_sickLeave->setRange(0, 30);
    m_sickLeave->setValue(7);
    leaveForm->addRow("Sick Leave:", m_sickLeave);

    m_earnedLeave = new QSpinBox;
    m_earnedLeave->setRange(0, 30);
    m_earnedLeave->setValue(15);
    leaveForm->addRow("Earned Leave:", m_earnedLeave);

    scrollLayout->addWidget(leaveGroup);

    scrollLayout->addStretch();

    scrollArea->setWidget(scrollWidget);
    mainLayout->addWidget(scrollArea, 1);

    // Status and Save
    m_statusLabel = new QLabel;
    m_statusLabel->setStyleSheet("color: #4ADE80; font-size: 13px; font-weight: 600;");
    mainLayout->addWidget(m_statusLabel);

    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();

    auto* saveBtn = new QPushButton("Save Settings");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(160, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &SettingsWidget::saveSettings);
    btnRow->addWidget(saveBtn);

    mainLayout->addLayout(btnRow);
}

void SettingsWidget::refresh() { loadSettings(); }

void SettingsWidget::loadSettings()
{
    auto& db = DatabaseManager::instance();
    auto getKey = [&](const QString& key, const QString& def) -> QString {
        auto q = db.execute("SELECT value FROM Settings WHERE key = :k", {{":k", key}});
        return q.next() ? q.value("value").toString() : def;
    };

    m_companyName->setText(getKey("company_name", ""));
    m_companyAddress->setText(getKey("company_address", ""));
    m_companyPhone->setText(getKey("company_phone", ""));
    m_companyEmail->setText(getKey("company_email", ""));

    m_pfRate->setValue(getKey("pf_rate", "12").toDouble());
    m_esicRate->setValue(getKey("esic_rate", "0.75").toDouble());
    m_ptDeduction->setValue(getKey("pt_deduction", "200").toDouble());
    m_overtimeRate->setValue(getKey("overtime_rate", "1.5").toDouble());
    m_workingDays->setValue(getKey("working_days", "26").toInt());

    m_casualLeave->setValue(getKey("casual_leave", "12").toInt());
    m_sickLeave->setValue(getKey("sick_leave", "7").toInt());
    m_earnedLeave->setValue(getKey("earned_leave", "15").toInt());

    m_statusLabel->setText("");
}

void SettingsWidget::saveSettings()
{
    auto& db = DatabaseManager::instance();
    auto setKey = [&](const QString& key, const QString& value) {
        db.executeNonQuery(
            "INSERT INTO Settings (key, value, updated_at) VALUES (:k, :v, datetime('now','localtime')) "
            "ON CONFLICT(key) DO UPDATE SET value = :v, updated_at = datetime('now','localtime')",
            {{":k", key}, {":v", value}}
        );
    };

    setKey("company_name", m_companyName->text().trimmed());
    setKey("company_address", m_companyAddress->text().trimmed());
    setKey("company_phone", m_companyPhone->text().trimmed());
    setKey("company_email", m_companyEmail->text().trimmed());

    setKey("pf_rate", QString::number(m_pfRate->value()));
    setKey("esic_rate", QString::number(m_esicRate->value()));
    setKey("pt_deduction", QString::number(m_ptDeduction->value()));
    setKey("overtime_rate", QString::number(m_overtimeRate->value()));
    setKey("working_days", QString::number(m_workingDays->value()));

    setKey("casual_leave", QString::number(m_casualLeave->value()));
    setKey("sick_leave", QString::number(m_sickLeave->value()));
    setKey("earned_leave", QString::number(m_earnedLeave->value()));

    m_statusLabel->setStyleSheet("color: #4ADE80; font-size: 13px; font-weight: 600;");
    m_statusLabel->setText("Settings saved successfully.");
}
