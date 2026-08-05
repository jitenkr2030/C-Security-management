#include "ComplianceWidget.h"
#include "ComplianceDialog.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>
#include <QScrollArea>
#include <QFrame>

ComplianceWidget::ComplianceWidget(QWidget* parent) : QWidget(parent) { buildUI(); refresh(); }

void ComplianceWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Compliance Management");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);
    auto* subtitle = new QLabel("PSARA, EPF, ESIC, Professional Tax, Minimum Wages, Bonus, Gratuity, POSH");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    m_tabs = new QTabWidget;
    m_tabs->addTab(buildDashboardTab(), "Dashboard");
    m_tabs->addTab(buildFilingsTab(), "Filing Tracker");
    m_tabs->addTab(buildLicensesTab(), "Licenses & Registrations");
    m_tabs->addTab(buildMinWagesTab(), "Minimum Wages");
    m_tabs->addTab(buildChecklistTab(), "Audit Checklist");
    mainLayout->addWidget(m_tabs, 1);
}

QWidget* ComplianceWidget::buildDashboardTab()
{
    auto* widget = new QWidget;
    auto* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    m_alertsLabel = new QLabel;
    m_alertsLabel->setStyleSheet("background-color: #2A1A1A; border: 1px solid #4A2020; border-radius: 8px; padding: 12px 16px; color: #E85454; font-weight: 700; font-size: 14px;");
    m_alertsLabel->setWordWrap(true);
    layout->addWidget(m_alertsLabel);

    auto addStatusCard = [&](const QString& title, QLabel*& label) {
        auto* card = new QGroupBox(title);
        card->setObjectName("DashCard");
        card->setMinimumHeight(80);
        auto* cl = new QVBoxLayout(card);
        label = new QLabel("Loading...");
        label->setStyleSheet("color: #8B95A5; font-size: 12px;");
        label->setWordWrap(true);
        cl->addWidget(label);
        return card;
    };

    auto* grid = new QGridLayout;
    grid->setSpacing(12);
    grid->addWidget(addStatusCard("PSARA License", m_psaraLabel), 0, 0);
    grid->addWidget(addStatusCard("EPF (Provident Fund)", m_epfLabel), 0, 1);
    grid->addWidget(addStatusCard("ESIC (Insurance)", m_esicLabel), 0, 2);
    grid->addWidget(addStatusCard("Professional Tax", m_ptLabel), 1, 0);
    grid->addWidget(addStatusCard("Bonus Act", m_bonusLabel), 1, 1);
    grid->addWidget(addStatusCard("Gratuity Act", m_gratuityLabel), 1, 2);
    grid->addWidget(addStatusCard("POSH Act", m_poshLabel), 2, 0);
    grid->addWidget(addStatusCard("Minimum Wages", m_wageLabel), 2, 1);
    layout->addLayout(grid, 1);

    return widget;
}

QWidget* ComplianceWidget::buildFilingsTab()
{
    auto* widget = new QWidget;
    auto* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(8);
    auto* addBtn = new QPushButton("+ Add Filing");
    addBtn->setObjectName("PrimaryButton");
    addBtn->setFixedSize(120, 34);
    addBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &ComplianceWidget::addFiling);
    headerRow->addWidget(addBtn);
    auto* editBtn = new QPushButton("Edit");
    editBtn->setObjectName("SecondaryButton");
    editBtn->setFixedSize(70, 34);
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, &ComplianceWidget::editFiling);
    headerRow->addWidget(editBtn);
    auto* delBtn = new QPushButton("Delete");
    delBtn->setObjectName("DangerButton");
    delBtn->setFixedSize(80, 34);
    delBtn->setCursor(Qt::PointingHandCursor);
    connect(delBtn, &QPushButton::clicked, this, &ComplianceWidget::deleteFiling);
    headerRow->addWidget(delBtn);
    headerRow->addStretch();
    m_filingSummary = new QLabel;
    m_filingSummary->setStyleSheet("color: #D4B44C; font-size: 13px; font-weight: 600;");
    headerRow->addWidget(m_filingSummary);
    layout->addLayout(headerRow);

    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(8);
    m_filingSearch = new QLineEdit;
    m_filingSearch->setPlaceholderText("Search filings...");
    m_filingSearch->setClearButtonEnabled(true);
    connect(m_filingSearch, &QLineEdit::textChanged, this, &ComplianceWidget::filterFilings);
    filterRow->addWidget(m_filingSearch, 1);
    m_filingAreaFilter = new QComboBox;
    m_filingAreaFilter->addItems({"All", "EPF", "ESIC", "Professional Tax", "Bonus", "Gratuity", "POSH", "PSARA", "Contract Labour"});
    m_filingAreaFilter->setFixedWidth(150);
    connect(m_filingAreaFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterFilings(); });
    filterRow->addWidget(m_filingAreaFilter);
    m_filingStatusFilter = new QComboBox;
    m_filingStatusFilter->addItems({"All", "Pending", "Filed", "Late", "Overdue"});
    m_filingStatusFilter->setFixedWidth(110);
    connect(m_filingStatusFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterFilings(); });
    filterRow->addWidget(m_filingStatusFilter);
    layout->addLayout(filterRow);

    m_filingTable = new QTableWidget;
    m_filingTable->setAlternatingRowColors(true);
    m_filingTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_filingTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_filingTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_filingTable->verticalHeader()->setVisible(false);
    m_filingTable->setShowGrid(false);
    m_filingTable->setSortingEnabled(true);
    QStringList cols = {"ID", "Area", "Type", "Period", "Due Date", "Filed Date", "Amount", "Status"};
    m_filingTable->setColumnCount(cols.size());
    m_filingTable->setHorizontalHeaderLabels(cols);
    m_filingTable->setColumnHidden(0, true);
    m_filingTable->setColumnWidth(1, 130);
    m_filingTable->setColumnWidth(2, 150);
    m_filingTable->setColumnWidth(3, 120);
    m_filingTable->setColumnWidth(4, 100);
    m_filingTable->setColumnWidth(5, 100);
    m_filingTable->setColumnWidth(6, 100);
    m_filingTable->horizontalHeader()->setStretchLastSection(true);
    connect(m_filingTable, &QTableWidget::cellDoubleClicked, this, &ComplianceWidget::editFiling);
    layout->addWidget(m_filingTable, 1);

    return widget;
}

QWidget* ComplianceWidget::buildLicensesTab()
{
    auto* widget = new QWidget;
    auto* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(8);
    auto* addBtn = new QPushButton("+ Add License");
    addBtn->setObjectName("PrimaryButton");
    addBtn->setFixedSize(140, 34);
    addBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &ComplianceWidget::addLicense);
    headerRow->addWidget(addBtn);
    auto* editBtn = new QPushButton("Edit");
    editBtn->setObjectName("SecondaryButton");
    editBtn->setFixedSize(70, 34);
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, &ComplianceWidget::editLicense);
    headerRow->addWidget(editBtn);
    auto* delBtn = new QPushButton("Delete");
    delBtn->setObjectName("DangerButton");
    delBtn->setFixedSize(80, 34);
    delBtn->setCursor(Qt::PointingHandCursor);
    connect(delBtn, &QPushButton::clicked, this, &ComplianceWidget::deleteLicense);
    headerRow->addWidget(delBtn);
    headerRow->addStretch();
    m_licenseSummary = new QLabel;
    m_licenseSummary->setStyleSheet("color: #D4B44C; font-size: 13px; font-weight: 600;");
    headerRow->addWidget(m_licenseSummary);
    layout->addLayout(headerRow);

    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(8);
    m_licenseSearch = new QLineEdit;
    m_licenseSearch->setPlaceholderText("Search licenses...");
    m_licenseSearch->setClearButtonEnabled(true);
    connect(m_licenseSearch, &QLineEdit::textChanged, this, &ComplianceWidget::filterLicenses);
    filterRow->addWidget(m_licenseSearch, 1);
    m_licenseTypeFilter = new QComboBox;
    m_licenseTypeFilter->addItems({"All", "PSARA License", "Contractor License", "EPF Registration",
                                   "ESIC Registration", "PT Registration", "Shops & Establishments",
                                   "Trade License", "Fire NOC", "Police Verification", "Other"});
    m_licenseTypeFilter->setFixedWidth(200);
    connect(m_licenseTypeFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterLicenses(); });
    filterRow->addWidget(m_licenseTypeFilter);
    layout->addLayout(filterRow);

    m_licenseTable = new QTableWidget;
    m_licenseTable->setAlternatingRowColors(true);
    m_licenseTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_licenseTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_licenseTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_licenseTable->verticalHeader()->setVisible(false);
    m_licenseTable->setShowGrid(false);
    m_licenseTable->setSortingEnabled(true);
    QStringList cols = {"ID", "Type", "Number", "Authority", "State", "Issue Date", "Expiry Date", "Status"};
    m_licenseTable->setColumnCount(cols.size());
    m_licenseTable->setHorizontalHeaderLabels(cols);
    m_licenseTable->setColumnHidden(0, true);
    m_licenseTable->setColumnWidth(1, 160);
    m_licenseTable->setColumnWidth(2, 140);
    m_licenseTable->setColumnWidth(3, 160);
    m_licenseTable->setColumnWidth(4, 110);
    m_licenseTable->setColumnWidth(5, 100);
    m_licenseTable->setColumnWidth(6, 100);
    m_licenseTable->horizontalHeader()->setStretchLastSection(true);
    connect(m_licenseTable, &QTableWidget::cellDoubleClicked, this, &ComplianceWidget::editLicense);
    layout->addWidget(m_licenseTable, 1);

    return widget;
}

QWidget* ComplianceWidget::buildMinWagesTab()
{
    auto* widget = new QWidget;
    auto* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(8);
    auto* addBtn = new QPushButton("+ Add Wage Rate");
    addBtn->setObjectName("PrimaryButton");
    addBtn->setFixedSize(160, 34);
    addBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &ComplianceWidget::addMinWage);
    headerRow->addWidget(addBtn);
    auto* editBtn = new QPushButton("Edit");
    editBtn->setObjectName("SecondaryButton");
    editBtn->setFixedSize(70, 34);
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, &ComplianceWidget::editMinWage);
    headerRow->addWidget(editBtn);
    auto* delBtn = new QPushButton("Delete");
    delBtn->setObjectName("DangerButton");
    delBtn->setFixedSize(80, 34);
    delBtn->setCursor(Qt::PointingHandCursor);
    connect(delBtn, &QPushButton::clicked, this, &ComplianceWidget::deleteMinWage);
    headerRow->addWidget(delBtn);
    headerRow->addStretch();
    m_wageSummary = new QLabel;
    m_wageSummary->setStyleSheet("color: #D4B44C; font-size: 13px; font-weight: 600;");
    headerRow->addWidget(m_wageSummary);
    layout->addLayout(headerRow);

    m_wageTable = new QTableWidget;
    m_wageTable->setAlternatingRowColors(true);
    m_wageTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_wageTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_wageTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_wageTable->verticalHeader()->setVisible(false);
    m_wageTable->setShowGrid(false);
    m_wageTable->setSortingEnabled(true);
    QStringList cols = {"ID", "State", "Zone", "Skill", "Basic", "VDA", "Total", "Effective From", "Notification"};
    m_wageTable->setColumnCount(cols.size());
    m_wageTable->setHorizontalHeaderLabels(cols);
    m_wageTable->setColumnHidden(0, true);
    m_wageTable->setColumnWidth(1, 130);
    m_wageTable->setColumnWidth(2, 80);
    m_wageTable->setColumnWidth(3, 110);
    m_wageTable->setColumnWidth(4, 90);
    m_wageTable->setColumnWidth(5, 80);
    m_wageTable->setColumnWidth(6, 90);
    m_wageTable->setColumnWidth(7, 110);
    m_wageTable->horizontalHeader()->setStretchLastSection(true);
    connect(m_wageTable, &QTableWidget::cellDoubleClicked, this, &ComplianceWidget::editMinWage);
    layout->addWidget(m_wageTable, 1);

    return widget;
}

QWidget* ComplianceWidget::buildChecklistTab()
{
    auto* widget = new QWidget;
    auto* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    auto* titleLabel = new QLabel("Compliance Audit Checklist");
    titleLabel->setStyleSheet("color: #D4B44C; font-weight: 700; font-size: 16px; padding: 4px 0;");
    layout->addWidget(titleLabel);

    auto* desc = new QLabel("Use this checklist to verify your agency's compliance status. Check each item during audits.");
    desc->setStyleSheet("color: #8B95A5; font-size: 12px; padding-bottom: 8px;");
    desc->setWordWrap(true);
    layout->addWidget(desc);

    m_checklistTable = new QTableWidget;
    m_checklistTable->setAlternatingRowColors(true);
    m_checklistTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_checklistTable->verticalHeader()->setVisible(false);
    m_checklistTable->setShowGrid(false);
    QStringList cols = {"#", "Compliance Area", "Checklist Item", "Legal Reference", "Status"};
    m_checklistTable->setColumnCount(cols.size());
    m_checklistTable->setHorizontalHeaderLabels(cols);
    m_checklistTable->setColumnWidth(0, 40);
    m_checklistTable->setColumnWidth(1, 150);
    m_checklistTable->setColumnWidth(2, 400);
    m_checklistTable->setColumnWidth(3, 200);
    m_checklistTable->horizontalHeader()->setStretchLastSection(true);

    struct Item { QString area; QString check; QString ref; };
    QList<Item> items = {
        {"PSARA", "PSARA license obtained for each state of operation", "PSARA Act 2005, Sec 4"},
        {"PSARA", "License renewed before expiry (45 days prior application)", "PSARA Act 2005, Sec 6"},
        {"PSARA", "Guards completed PSARA-mandated training (min 100 hours)", "PSARA Act 2005, Sec 10"},
        {"PSARA", "Character verification of all guards completed", "PSARA Act 2005, Sec 7"},
        {"PSARA", "Display license at registered office", "PSARA Act 2005, Sec 5"},
        {"EPF", "EPF registration obtained (20+ employees)", "EPF Act 1952, Sec 1"},
        {"EPF", "ECR filed by 15th of every month", "EPF Scheme, Para 36"},
        {"EPF", "Employee contribution (12%) deducted correctly", "EPF Scheme, Para 29"},
        {"EPF", "Employer contribution (12%) deposited", "EPF Scheme, Para 29"},
        {"EPF", "Annual return filed by April 30", "EPF Act, Sec 8"},
        {"ESIC", "ESIC registration obtained (10+ employees)", "ESI Act 1948, Sec 1"},
        {"ESIC", "Contribution paid by 15th of following month", "ESI Act, Sec 39"},
        {"ESIC", "Employee contribution (0.75%) deducted", "ESI Act, Sec 39(2)"},
        {"ESIC", "Employer contribution (3.25%) deposited", "ESI Act, Sec 39(2)"},
        {"ESIC", "Half-yearly return filed within 2 months", "ESI Reg 82"},
        {"PT", "PT registration obtained for each state", "State PT Act"},
        {"PT", "Monthly PT deducted and deposited", "State PT Act"},
        {"PT", "Annual return filed", "State PT Act"},
        {"Min Wages", "All guards paid at or above state minimum wage", "Min Wages Act 1948, Sec 5"},
        {"Min Wages", "Wages updated after government notification", "Min Wages Act 1948, Sec 5"},
        {"Min Wages", "Wage registers maintained", "Min Wages Act 1948, Sec 18"},
        {"Min Wages", "Overtime paid at double rate", "Min Wages Act 1948, Sec 14"},
        {"Bonus", "Minimum bonus (8.33%) paid to eligible employees", "Payment of Bonus Act 1965, Sec 10"},
        {"Bonus", "Bonus paid before October 31", "Payment of Bonus Act 1965, Sec 19"},
        {"Bonus", "Bonus register maintained", "Payment of Bonus Act 1965, Sec 26"},
        {"Gratuity", "Gratuity paid to eligible employees (5+ years)", "Payment of Gratuity Act 1972, Sec 4"},
        {"Gratuity", "Gratuity nominations collected from all employees", "Payment of Gratuity Act 1972, Sec 6"},
        {"Gratuity", "Gratuity insurance policy obtained (if applicable)", "Payment of Gratuity Act 1972, Sec 4A"},
        {"POSH", "ICC constituted with min 4 members, 50% women", "POSH Act 2013, Sec 4"},
        {"POSH", "Anti-sexual harassment policy circulated", "POSH Act 2013, Sec 4(1)"},
        {"POSH", "Annual POSH training conducted for employees", "POSH Act 2013, Sec 19(c)"},
        {"POSH", "Annual report filed with District Officer", "POSH Act 2013, Sec 21"},
        {"S&E Act", "Shops & Establishments registration obtained", "State S&E Act"},
        {"S&E Act", "Working hours within limits (max 8 hrs/day, 48 hrs/week)", "State S&E Act"},
        {"S&E Act", "Weekly holiday provided", "State S&E Act"},
        {"S&E Act", "Leave records maintained as per state act", "State S&E Act"},
        {"Contract Labour", "Contractor license obtained (20+ contract workers)", "CLRA Act 1970, Sec 12"},
        {"Contract Labour", "Principal employer registration done", "CLRA Act 1970, Sec 7"},
        {"Contract Labour", "Welfare facilities provided (canteen, water, first aid)", "CLRA Act 1970, Sec 21-24"},
        {"Contract Labour", "License renewed before expiry", "CLRA Act 1970, Sec 14"},
    };

    m_checklistTable->setRowCount(items.size());
    for (int i = 0; i < items.size(); ++i) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_checklistTable->setItem(i, col, item);
        };
        setItem(0, QString::number(i + 1));
        auto* areaItem = new QTableWidgetItem(items[i].area);
        areaItem->setForeground(QColor("#D4B44C"));
        areaItem->setFont(QFont("", -1, QFont::Bold));
        m_checklistTable->setItem(i, 1, areaItem);
        setItem(2, items[i].check);
        setItem(3, items[i].ref);
        auto* statusItem = new QTableWidgetItem("Check");
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setForeground(QColor("#FBBF24"));
        m_checklistTable->setItem(i, 4, statusItem);
    }

    layout->addWidget(m_checklistTable, 1);
    return widget;
}

void ComplianceWidget::loadDashboard()
{
    auto& db = DatabaseManager::instance();
    QDate today = QDate::currentDate();

    // PSARA license status
    auto psara = db.execute("SELECT license_number, expiry_date, status FROM ComplianceLicenses WHERE license_type LIKE '%PSARA%' ORDER BY expiry_date DESC LIMIT 1");
    if (psara.next()) {
        QDate expiry = QDate::fromString(psara.value("expiry_date").toString(), "yyyy-MM-dd");
        int daysLeft = today.daysTo(expiry);
        QString status = psara.value("status").toString();
        if (status == "Expired" || daysLeft < 0) {
            m_psaraLabel->setText(QString("EXPIRED - License: %1").arg(psara.value("license_number").toString()));
            m_psaraLabel->setStyleSheet("color: #E85454; font-weight: 700; font-size: 12px;");
        } else if (daysLeft <= 90) {
            m_psaraLabel->setText(QString("EXPIRES in %1 days - %2").arg(daysLeft).arg(psara.value("license_number").toString()));
            m_psaraLabel->setStyleSheet("color: #FB923C; font-weight: 700; font-size: 12px;");
        } else {
            m_psaraLabel->setText(QString("Active - Expires: %1 (%2 days)").arg(expiry.toString("yyyy-MM-dd")).arg(daysLeft));
            m_psaraLabel->setStyleSheet("color: #4ADE80; font-size: 12px;");
        }
    } else {
        m_psaraLabel->setText("No PSARA license recorded");
        m_psaraLabel->setStyleSheet("color: #E85454; font-weight: 700; font-size: 12px;");
    }

    // EPF filing status
    auto epf = db.execute("SELECT due_date, status, amount FROM ComplianceFilings WHERE compliance_area = 'EPF' ORDER BY due_date DESC LIMIT 1");
    if (epf.next()) {
        QDate due = QDate::fromString(epf.value("due_date").toString(), "yyyy-MM-dd");
        int daysLeft = today.daysTo(due);
        QString status = epf.value("status").toString();
        if (status == "Filed") {
            m_epfLabel->setText(QString("Last filed - Rs. %1").arg(epf.value("amount").toDouble(), 0, 'f', 0));
            m_epfLabel->setStyleSheet("color: #4ADE80; font-size: 12px;");
        } else {
            m_epfLabel->setText(QString("%1 - Rs. %2 (Due: %3)").arg(status, QString::number(epf.value("amount").toDouble(), 'f', 0), due.toString("yyyy-MM-dd")));
            m_epfLabel->setStyleSheet(status == "Overdue" ? "color: #E85454; font-weight: 700; font-size: 12px;" : "color: #FBBF24; font-size: 12px;");
        }
    } else {
        m_epfLabel->setText("No EPF filings recorded");
        m_epfLabel->setStyleSheet("color: #8B95A5; font-size: 12px;");
    }

    // ESIC filing status
    auto esic = db.execute("SELECT due_date, status, amount FROM ComplianceFilings WHERE compliance_area = 'ESIC' ORDER BY due_date DESC LIMIT 1");
    if (esic.next()) {
        QString status = esic.value("status").toString();
        QDate due = QDate::fromString(esic.value("due_date").toString(), "yyyy-MM-dd");
        m_esicLabel->setText(QString("%1 - Rs. %2 (Due: %3)").arg(status, QString::number(esic.value("amount").toDouble(), 'f', 0), due.toString("yyyy-MM-dd")));
        m_esicLabel->setStyleSheet(status == "Filed" ? "color: #4ADE80; font-size: 12px;" : "color: #FBBF24; font-size: 12px;");
    } else {
        m_esicLabel->setText("No ESIC filings recorded");
        m_esicLabel->setStyleSheet("color: #8B95A5; font-size: 12px;");
    }

    // PT status
    auto pt = db.execute("SELECT due_date, status, amount FROM ComplianceFilings WHERE compliance_area = 'Professional Tax' ORDER BY due_date DESC LIMIT 1");
    if (pt.next()) {
        QString status = pt.value("status").toString();
        m_ptLabel->setText(QString("%1 - Rs. %2").arg(status, QString::number(pt.value("amount").toDouble(), 'f', 0)));
        m_ptLabel->setStyleSheet(status == "Filed" ? "color: #4ADE80; font-size: 12px;" : "color: #FBBF24; font-size: 12px;");
    } else {
        m_ptLabel->setText("No PT filings recorded");
        m_ptLabel->setStyleSheet("color: #8B95A5; font-size: 12px;");
    }

    // Bonus
    auto bonus = db.execute("SELECT status, amount FROM ComplianceFilings WHERE compliance_area = 'Bonus' ORDER BY due_date DESC LIMIT 1");
    if (bonus.next()) {
        m_bonusLabel->setText(QString("%1 - Rs. %2").arg(bonus.value("status").toString(), QString::number(bonus.value("amount").toDouble(), 'f', 0)));
        m_bonusLabel->setStyleSheet(bonus.value("status").toString() == "Filed" ? "color: #4ADE80; font-size: 12px;" : "color: #FBBF24; font-size: 12px;");
    } else {
        m_bonusLabel->setText("No bonus filings recorded");
        m_bonusLabel->setStyleSheet("color: #8B95A5; font-size: 12px;");
    }

    // Gratuity
    auto gratQ = db.execute("SELECT COUNT(*) FROM Guards WHERE status = 'Active' AND julianday('now') - julianday(join_date) > 1825");
    int eligible = 0;
    if (gratQ.next()) eligible = gratQ.value(0).toInt();
    m_gratuityLabel->setText(QString("%1 eligible employees (5+ years)").arg(eligible));
    m_gratuityLabel->setStyleSheet(eligible > 0 ? "color: #60A5FA; font-size: 12px;" : "color: #4ADE80; font-size: 12px;");

    // POSH
    m_poshLabel->setText("ICC Status: Verify manually");
    m_poshLabel->setStyleSheet("color: #FBBF24; font-size: 12px;");

    // Min Wages
    auto wages = db.execute("SELECT COUNT(*) FROM MinWages");
    int wageCount = 0;
    if (wages.next()) wageCount = wages.value(0).toInt();
    m_wageLabel->setText(wageCount > 0 ? QString("%1 wage rates configured").arg(wageCount) : "No minimum wage rates configured");
    m_wageLabel->setStyleSheet(wageCount > 0 ? "color: #4ADE80; font-size: 12px;" : "color: #E85454; font-weight: 700; font-size: 12px;");

    // Alerts summary
    int alerts = 0;
    auto overdueFilings = db.execute("SELECT COUNT(*) FROM ComplianceFilings WHERE status = 'Pending' AND due_date < date('now','localtime')");
    if (overdueFilings.next()) alerts += overdueFilings.value(0).toInt();
    auto expiringSoon = db.execute("SELECT COUNT(*) FROM ComplianceLicenses WHERE status = 'Active' AND julianday(expiry_date) - julianday('now','localtime') < 90");
    if (expiringSoon.next()) alerts += expiringSoon.value(0).toInt();
    auto expiredLic = db.execute("SELECT COUNT(*) FROM ComplianceLicenses WHERE expiry_date < date('now','localtime') AND status = 'Active'");
    if (expiredLic.next()) alerts += expiredLic.value(0).toInt();

    if (alerts > 0) {
        m_alertsLabel->setText(QString("  %1 item(s) need immediate attention! Check filings and licenses.").arg(alerts));
        m_alertsLabel->setStyleSheet("background-color: #2A1A1A; border: 1px solid #4A2020; border-radius: 8px; padding: 12px 16px; color: #E85454; font-weight: 700; font-size: 14px;");
    } else {
        m_alertsLabel->setText("  All compliance items are up to date.");
        m_alertsLabel->setStyleSheet("background-color: #1A2A1A; border: 1px solid #2A4A2A; border-radius: 8px; padding: 12px 16px; color: #4ADE80; font-weight: 700; font-size: 14px;");
    }
}

void ComplianceWidget::loadFilings()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute("SELECT * FROM ComplianceFilings ORDER BY due_date DESC");

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM ComplianceFilings");
    if (cc.next()) count = cc.value(0).toInt();
    m_filingTable->setRowCount(count);
    m_filingTable->setSortingEnabled(false);

    int row = 0, pending = 0, filed = 0, overdue = 0;
    double totalAmount = 0;

    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_filingTable->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        setItem(1, query.value("compliance_area").toString());
        setItem(2, query.value("filing_type").toString());
        setItem(3, query.value("filing_period").toString());
        setItem(4, query.value("due_date").toString());
        setItem(5, query.value("filed_date").toString());

        double amt = query.value("amount").toDouble();
        totalAmount += amt;
        auto* amtItem = new QTableWidgetItem(QString("Rs. %1").arg(amt, 0, 'f', 0));
        amtItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_filingTable->setItem(row, 6, amtItem);

        // Auto-update overdue status
        QString status = query.value("status").toString();
        QDate due = QDate::fromString(query.value("due_date").toString(), "yyyy-MM-dd");
        if (status == "Pending" && due < QDate::currentDate()) {
            status = "Overdue";
            db.executeNonQuery("UPDATE ComplianceFilings SET status = 'Overdue' WHERE id = :id",
                               {{":id", query.value("id").toInt()}});
        }

        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "Filed") { statusItem->setForeground(QColor("#4ADE80")); filed++; }
        else if (status == "Pending") { statusItem->setForeground(QColor("#FBBF24")); pending++; }
        else if (status == "Late") { statusItem->setForeground(QColor("#FB923C")); }
        else if (status == "Overdue") { statusItem->setForeground(QColor("#E85454")); overdue++; }
        m_filingTable->setItem(row, 7, statusItem);

        row++;
    }
    m_filingTable->setSortingEnabled(true);
    m_filingSummary->setText(QString("%1 records | Filed: %2 | Pending: %3 | Overdue: %4 | Total: Rs. %5")
        .arg(row).arg(filed).arg(pending).arg(overdue).arg(totalAmount, 0, 'f', 0));
}

void ComplianceWidget::loadLicenses()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute("SELECT * FROM ComplianceLicenses ORDER BY expiry_date ASC");

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM ComplianceLicenses");
    if (cc.next()) count = cc.value(0).toInt();
    m_licenseTable->setRowCount(count);
    m_licenseTable->setSortingEnabled(false);

    int row = 0, active = 0, expiring = 0, expired = 0;
    QDate today = QDate::currentDate();

    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_licenseTable->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        setItem(1, query.value("license_type").toString());
        setItem(2, query.value("license_number").toString());
        setItem(3, query.value("issuing_authority").toString());
        setItem(4, query.value("state").toString());
        setItem(5, query.value("issue_date").toString());

        QDate expiry = QDate::fromString(query.value("expiry_date").toString(), "yyyy-MM-dd");
        int daysLeft = today.daysTo(expiry);
        auto* expiryItem = new QTableWidgetItem(query.value("expiry_date").toString());
        expiryItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        if (daysLeft < 0) expiryItem->setForeground(QColor("#E85454"));
        else if (daysLeft <= 90) expiryItem->setForeground(QColor("#FB923C"));
        m_licenseTable->setItem(row, 6, expiryItem);

        QString status = query.value("status").toString();
        if (status == "Active" && daysLeft < 0) status = "Expired";
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "Active") { statusItem->setForeground(QColor("#4ADE80")); active++; }
        else if (status == "Under Renewal") { statusItem->setForeground(QColor("#FBBF24")); expiring++; }
        else if (status == "Expired") { statusItem->setForeground(QColor("#E85454")); expired++; }
        else statusItem->setForeground(QColor("#6B7585"));
        m_licenseTable->setItem(row, 7, statusItem);

        row++;
    }
    m_licenseTable->setSortingEnabled(true);
    m_licenseSummary->setText(QString("%1 licenses | Active: %2 | Expiring Soon: %3 | Expired: %4").arg(row).arg(active).arg(expiring).arg(expired));
}

void ComplianceWidget::loadMinWages()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute("SELECT * FROM MinWages ORDER BY state, zone, skill_category");

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM MinWages");
    if (cc.next()) count = cc.value(0).toInt();
    m_wageTable->setRowCount(count);
    m_wageTable->setSortingEnabled(false);

    int row = 0;
    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_wageTable->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        setItem(1, query.value("state").toString());
        setItem(2, query.value("zone").toString());
        setItem(3, query.value("skill_category").toString());

        auto moneyItem = [&](int col, double val) {
            auto* item = new QTableWidgetItem(QString("Rs. %1").arg(val, 0, 'f', 0));
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_wageTable->setItem(row, col, item);
        };
        moneyItem(4, query.value("basic_wage").toDouble());
        moneyItem(5, query.value("vda").toDouble());

        auto* totalItem = new QTableWidgetItem(QString("Rs. %1").arg(query.value("total_wage").toDouble(), 0, 'f', 0));
        totalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        totalItem->setForeground(QColor("#D4B44C"));
        totalItem->setFont(QFont("", -1, QFont::Bold));
        m_wageTable->setItem(row, 6, totalItem);

        setItem(7, query.value("effective_from").toString());
        setItem(8, query.value("notification_no").toString());
        row++;
    }
    m_wageTable->setSortingEnabled(true);
    m_wageSummary->setText(QString("%1 wage rate entries").arg(row));
}

void ComplianceWidget::refresh()
{
    loadDashboard();
    loadFilings();
    loadLicenses();
    loadMinWages();
}

void ComplianceWidget::addFiling() { ComplianceDialog dlg(this, ComplianceDialog::FilingType); if (dlg.exec() == QDialog::Accepted) { loadFilings(); loadDashboard(); } }
void ComplianceWidget::editFiling() {
    auto items = m_filingTable->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a filing to edit."); return; }
    int id = m_filingTable->item(items.first()->row(), 0)->text().toInt();
    ComplianceDialog dlg(this, ComplianceDialog::FilingType, id);
    if (dlg.exec() == QDialog::Accepted) { loadFilings(); loadDashboard(); }
}
void ComplianceWidget::deleteFiling() {
    auto items = m_filingTable->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a filing to delete."); return; }
    int id = m_filingTable->item(items.first()->row(), 0)->text().toInt();
    if (QMessageBox::question(this, "Delete", "Delete this filing record?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        DatabaseManager::instance().executeNonQuery("DELETE FROM ComplianceFilings WHERE id = :id", {{":id", id}});
        loadFilings(); loadDashboard();
    }
}

void ComplianceWidget::addLicense() { ComplianceDialog dlg(this, ComplianceDialog::LicenseType); if (dlg.exec() == QDialog::Accepted) { loadLicenses(); loadDashboard(); } }
void ComplianceWidget::editLicense() {
    auto items = m_licenseTable->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a license to edit."); return; }
    int id = m_licenseTable->item(items.first()->row(), 0)->text().toInt();
    ComplianceDialog dlg(this, ComplianceDialog::LicenseType, id);
    if (dlg.exec() == QDialog::Accepted) { loadLicenses(); loadDashboard(); }
}
void ComplianceWidget::deleteLicense() {
    auto items = m_licenseTable->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a license to delete."); return; }
    int id = m_licenseTable->item(items.first()->row(), 0)->text().toInt();
    if (QMessageBox::question(this, "Delete", "Delete this license?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        DatabaseManager::instance().executeNonQuery("DELETE FROM ComplianceLicenses WHERE id = :id", {{":id", id}});
        loadLicenses(); loadDashboard();
    }
}

void ComplianceWidget::addMinWage() { ComplianceDialog dlg(this, ComplianceDialog::MinWageType); if (dlg.exec() == QDialog::Accepted) loadMinWages(); }
void ComplianceWidget::editMinWage() {
    auto items = m_wageTable->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a wage rate to edit."); return; }
    int id = m_wageTable->item(items.first()->row(), 0)->text().toInt();
    ComplianceDialog dlg(this, ComplianceDialog::MinWageType, id);
    if (dlg.exec() == QDialog::Accepted) loadMinWages();
}
void ComplianceWidget::deleteMinWage() {
    auto items = m_wageTable->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a wage rate to delete."); return; }
    int id = m_wageTable->item(items.first()->row(), 0)->text().toInt();
    if (QMessageBox::question(this, "Delete", "Delete this wage rate?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        DatabaseManager::instance().executeNonQuery("DELETE FROM MinWages WHERE id = :id", {{":id", id}});
        loadMinWages();
    }
}

void ComplianceWidget::filterFilings()
{
    QString text = m_filingSearch->text().toLower();
    QString areaFilter = m_filingAreaFilter->currentText();
    QString statusFilter = m_filingStatusFilter->currentText();
    for (int row = 0; row < m_filingTable->rowCount(); ++row) {
        bool textMatch = text.isEmpty();
        bool areaMatch = (areaFilter == "All");
        bool statusMatch = (statusFilter == "All");
        if (!textMatch) { for (int col = 1; col <= 6; ++col) { auto* item = m_filingTable->item(row, col); if (item && item->text().toLower().contains(text)) { textMatch = true; break; } } }
        if (!areaMatch) { auto* item = m_filingTable->item(row, 1); if (item) areaMatch = (item->text() == areaFilter); }
        if (!statusMatch) { auto* item = m_filingTable->item(row, 7); if (item) statusMatch = (item->text() == statusFilter); }
        m_filingTable->setRowHidden(row, !(textMatch && areaMatch && statusMatch));
    }
}

void ComplianceWidget::filterLicenses()
{
    QString text = m_licenseSearch->text().toLower();
    QString typeFilter = m_licenseTypeFilter->currentText();
    for (int row = 0; row < m_licenseTable->rowCount(); ++row) {
        bool textMatch = text.isEmpty();
        bool typeMatch = (typeFilter == "All");
        if (!textMatch) { for (int col = 1; col <= 6; ++col) { auto* item = m_licenseTable->item(row, col); if (item && item->text().toLower().contains(text)) { textMatch = true; break; } } }
        if (!typeMatch) { auto* item = m_licenseTable->item(row, 1); if (item) typeMatch = (item->text() == typeFilter); }
        m_licenseTable->setRowHidden(row, !(textMatch && typeMatch));
    }
}

void ComplianceWidget::checkCompliance() { loadDashboard(); }
