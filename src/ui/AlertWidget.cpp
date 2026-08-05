#include "AlertWidget.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QCoreApplication>
#include <QDir>

struct AlertRow {
    QString category;
    QString priority;
    QString message;
    QString details;
    QString dueDate;
    int referenceId;
    QString status;
};

AlertWidget::AlertWidget(QWidget* parent) : QWidget(parent) { buildUI(); loadAlerts(); }

void AlertWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Alerts & Notifications");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);
    auto* subtitle = new QLabel("License expiry, compliance deadlines, pending salaries and active warnings");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(8);

    auto* refreshBtn = new QPushButton("Refresh Alerts");
    refreshBtn->setObjectName("PrimaryButton");
    refreshBtn->setFixedSize(140, 36);
    refreshBtn->setCursor(Qt::PointingHandCursor);
    connect(refreshBtn, &QPushButton::clicked, this, &AlertWidget::loadAlerts);
    headerRow->addWidget(refreshBtn);

    auto* ackBtn = new QPushButton("Acknowledge");
    ackBtn->setObjectName("SecondaryButton");
    ackBtn->setFixedSize(120, 36);
    ackBtn->setCursor(Qt::PointingHandCursor);
    connect(ackBtn, &QPushButton::clicked, this, &AlertWidget::acknowledgeAlert);
    headerRow->addWidget(ackBtn);

    auto* resolveBtn = new QPushButton("Dismiss");
    resolveBtn->setFixedSize(90, 36);
    resolveBtn->setCursor(Qt::PointingHandCursor);
    resolveBtn->setStyleSheet("QPushButton { background-color: #1A3A1A; color: #4ADE80; border: 1px solid #2A4A2A; border-radius: 6px; padding: 6px 16px; font-weight: 600; } QPushButton:hover { background-color: #2A4A2A; }");
    connect(resolveBtn, &QPushButton::clicked, this, &AlertWidget::resolveAlert);
    headerRow->addWidget(resolveBtn);

    auto* exportBtn = new QPushButton("Export CSV");
    exportBtn->setObjectName("SecondaryButton");
    exportBtn->setFixedSize(110, 36);
    exportBtn->setCursor(Qt::PointingHandCursor);
    connect(exportBtn, &QPushButton::clicked, this, &AlertWidget::exportCSV);
    headerRow->addWidget(exportBtn);

    headerRow->addStretch();
    m_countLabel = new QLabel;
    m_countLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    headerRow->addWidget(m_countLabel);
    mainLayout->addLayout(headerRow);

    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search alerts...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &AlertWidget::filterAlerts);
    filterRow->addWidget(m_searchEdit, 1);

    m_categoryFilter = new QComboBox;
    m_categoryFilter->addItems({"All", "License", "Compliance", "Salary", "Leave", "Complaint", "Incident"});
    m_categoryFilter->setFixedWidth(130);
    connect(m_categoryFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterAlerts(m_searchEdit->text()); });
    filterRow->addWidget(m_categoryFilter);

    m_priorityFilter = new QComboBox;
    m_priorityFilter->addItems({"All", "Critical", "High", "Medium", "Low"});
    m_priorityFilter->setFixedWidth(110);
    connect(m_priorityFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterAlerts(m_searchEdit->text()); });
    filterRow->addWidget(m_priorityFilter);
    mainLayout->addLayout(filterRow);

    m_summaryLabel = new QLabel;
    m_summaryLabel->setStyleSheet("color: #D4B44C; font-size: 13px; font-weight: 600;");
    mainLayout->addWidget(m_summaryLabel);

    m_table = new QTableWidget;
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(true);

    QStringList cols = {"#", "Category", "Priority", "Message", "Details", "Due Date", "Status"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnWidth(0, 40);
    m_table->setColumnWidth(1, 110);
    m_table->setColumnWidth(2, 80);
    m_table->setColumnWidth(3, 350);
    m_table->setColumnWidth(4, 250);
    m_table->setColumnWidth(5, 110);
    m_table->horizontalHeader()->setStretchLastSection(true);
    mainLayout->addWidget(m_table, 1);
}

void AlertWidget::loadAlerts()
{
    m_table->setSortingEnabled(false);
    m_table->setRowCount(0);

    scanLicenseAlerts();
    scanComplianceAlerts();
    scanSalaryAlerts();
    scanLeaveAlerts();
    scanComplaintAlerts();
    scanIncidentAlerts();

    m_table->setSortingEnabled(true);
    int total = m_table->rowCount();
    int critical = 0, high = 0;
    for (int r = 0; r < total; ++r) {
        auto* item = m_table->item(r, 2);
        if (item) {
            if (item->text() == "Critical") critical++;
            else if (item->text() == "High") high++;
        }
    }
    m_countLabel->setText(QString("%1 alerts").arg(total));
    m_summaryLabel->setText(QString("Active: %1 | Critical: %2 | High: %3").arg(total).arg(critical).arg(high));
}

void AlertWidget::scanLicenseAlerts()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute("SELECT * FROM License WHERE status = 'Active' AND expiry_date != '' AND expiry_date IS NOT NULL");
    QDate today = QDate::currentDate();

    while (query.next()) {
        QDate expiry = QDate::fromString(query.value("expiry_date").toString(), "yyyy-MM-dd");
        if (!expiry.isValid()) continue;
        int daysLeft = today.daysTo(expiry);
        if (daysLeft > 90) continue;

        int row = m_table->rowCount();
        m_table->insertRow(row);

        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));

        auto* catItem = new QTableWidgetItem("License");
        catItem->setForeground(QColor("#60A5FA"));
        m_table->setItem(row, 1, catItem);

        QString priority;
        QColor priColor;
        if (daysLeft <= 0) { priority = "Critical"; priColor = QColor("#E85454"); }
        else if (daysLeft <= 30) { priority = "High"; priColor = QColor("#FB923C"); }
        else if (daysLeft <= 60) { priority = "Medium"; priColor = QColor("#FBBF24"); }
        else { priority = "Low"; priColor = QColor("#60A5FA"); }
        auto* priItem = new QTableWidgetItem(priority);
        priItem->setForeground(priColor);
        priItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 2, priItem);

        QString lType = query.value("license_type").toString();
        QString lNum = query.value("license_number").toString();
        QString msg = daysLeft <= 0
            ? QString("%1 License EXPIRED: %2").arg(lType, lNum)
            : QString("%1 License expires in %2 days: %3").arg(lType).arg(daysLeft).arg(lNum);
        m_table->setItem(row, 3, new QTableWidgetItem(msg));

        m_table->setItem(row, 4, new QTableWidgetItem(
            "Authority: " + query.value("issuing_authority").toString() +
            " | State: " + query.value("state").toString()));

        m_table->setItem(row, 5, new QTableWidgetItem(expiry.toString("yyyy-MM-dd")));

        auto* statusItem = new QTableWidgetItem(daysLeft <= 0 ? "EXPIRED" : "Active");
        statusItem->setForeground(daysLeft <= 0 ? QColor("#E85454") : QColor("#FBBF24"));
        statusItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 6, statusItem);
    }
}

void AlertWidget::scanComplianceAlerts()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute("SELECT * FROM ComplianceFiling WHERE status = 'Pending'");
    QDate today = QDate::currentDate();

    while (query.next()) {
        QDate dueDate = QDate::fromString(query.value("due_date").toString(), "yyyy-MM-dd");
        if (!dueDate.isValid()) continue;
        int daysLeft = today.daysTo(dueDate);

        int row = m_table->rowCount();
        m_table->insertRow(row);

        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));

        auto* catItem = new QTableWidgetItem("Compliance");
        catItem->setForeground(QColor("#A78BFA"));
        m_table->setItem(row, 1, catItem);

        QString priority;
        QColor priColor;
        if (daysLeft < 0) { priority = "Critical"; priColor = QColor("#E85454"); }
        else if (daysLeft <= 3) { priority = "High"; priColor = QColor("#FB923C"); }
        else if (daysLeft <= 7) { priority = "Medium"; priColor = QColor("#FBBF24"); }
        else { priority = "Low"; priColor = QColor("#60A5FA"); }
        auto* priItem = new QTableWidgetItem(priority);
        priItem->setForeground(priColor);
        priItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 2, priItem);

        QString area = query.value("compliance_area").toString();
        QString fType = query.value("filing_type").toString();
        QString period = query.value("filing_period").toString();
        double amount = query.value("amount_paid").toDouble();
        QString msg = daysLeft < 0
            ? QString("%1 %2 OVERDUE for %3 (was due %4 days ago)").arg(area, fType, period).arg(-daysLeft)
            : QString("%1 %2 due for %3 in %4 days").arg(area, fType, period).arg(daysLeft);
        m_table->setItem(row, 3, new QTableWidgetItem(msg));
        m_table->setItem(row, 4, new QTableWidgetItem(
            "Amount: Rs. " + QString::number(amount, 'f', 0)));
        m_table->setItem(row, 5, new QTableWidgetItem(dueDate.toString("yyyy-MM-dd")));

        auto* statusItem = new QTableWidgetItem(daysLeft < 0 ? "OVERDUE" : "Pending");
        statusItem->setForeground(daysLeft < 0 ? QColor("#E85454") : QColor("#FBBF24"));
        statusItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 6, statusItem);
    }
}

void AlertWidget::scanSalaryAlerts()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute("SELECT COUNT(*) FROM Salary WHERE payment_status = 'Pending'");
    int pendingCount = 0;
    if (query.next()) pendingCount = query.value(0).toInt();

    if (pendingCount == 0) return;

    auto totalQ = db.execute("SELECT COALESCE(SUM(net_salary), 0) FROM Salary WHERE payment_status = 'Pending'");
    double totalPending = 0;
    if (totalQ.next()) totalPending = totalQ.value(0).toDouble();

    int row = m_table->rowCount();
    m_table->insertRow(row);

    m_table->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
    auto* catItem = new QTableWidgetItem("Salary");
    catItem->setForeground(QColor("#FBBF24"));
    m_table->setItem(row, 1, catItem);

    auto* priItem = new QTableWidgetItem(pendingCount > 5 ? "High" : "Medium");
    priItem->setForeground(pendingCount > 5 ? QColor("#FB923C") : QColor("#FBBF24"));
    priItem->setTextAlignment(Qt::AlignCenter);
    m_table->setItem(row, 2, priItem);

    m_table->setItem(row, 3, new QTableWidgetItem(
        QString("%1 guard salaries pending. Total: Rs. %2").arg(pendingCount).arg(totalPending, 0, 'f', 0)));
    m_table->setItem(row, 4, new QTableWidgetItem("Process payroll to clear pending salaries"));
    m_table->setItem(row, 5, new QTableWidgetItem("-"));

    auto* statusItem = new QTableWidgetItem("Pending");
    statusItem->setForeground(QColor("#FBBF24"));
    statusItem->setTextAlignment(Qt::AlignCenter);
    m_table->setItem(row, 6, statusItem);
}

void AlertWidget::scanLeaveAlerts()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute("SELECT COUNT(*) FROM LeaveRecord WHERE status = 'Pending'");
    int pending = 0;
    if (query.next()) pending = query.value(0).toInt();

    if (pending == 0) return;

    int row = m_table->rowCount();
    m_table->insertRow(row);

    m_table->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
    auto* catItem = new QTableWidgetItem("Leave");
    catItem->setForeground(QColor("#4ADE80"));
    m_table->setItem(row, 1, catItem);

    auto* priItem = new QTableWidgetItem("Medium");
    priItem->setForeground(QColor("#FBBF24"));
    priItem->setTextAlignment(Qt::AlignCenter);
    m_table->setItem(row, 2, priItem);

    m_table->setItem(row, 3, new QTableWidgetItem(
        QString("%1 leave requests pending approval").arg(pending)));
    m_table->setItem(row, 4, new QTableWidgetItem("Review and approve/reject leave requests"));
    m_table->setItem(row, 5, new QTableWidgetItem("-"));

    auto* statusItem = new QTableWidgetItem("Active");
    statusItem->setForeground(QColor("#FBBF24"));
    statusItem->setTextAlignment(Qt::AlignCenter);
    m_table->setItem(row, 6, statusItem);
}

void AlertWidget::scanComplaintAlerts()
{
    auto& db = DatabaseManager::instance();

    auto openQ = db.execute("SELECT COUNT(*) FROM Complaints WHERE status = 'Open'");
    int open = 0;
    if (openQ.next()) open = openQ.value(0).toInt();

    auto escQ = db.execute("SELECT COUNT(*) FROM Complaints WHERE status = 'Escalated'");
    int escalated = 0;
    if (escQ.next()) escalated = escQ.value(0).toInt();

    if (escalated > 0) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
        auto* catItem = new QTableWidgetItem("Complaint");
        catItem->setForeground(QColor("#E85454"));
        m_table->setItem(row, 1, catItem);
        auto* priItem = new QTableWidgetItem("Critical");
        priItem->setForeground(QColor("#E85454"));
        priItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 2, priItem);
        m_table->setItem(row, 3, new QTableWidgetItem(
            QString("%1 escalated complaints require immediate attention").arg(escalated)));
        m_table->setItem(row, 4, new QTableWidgetItem("Review escalated complaints and take action"));
        m_table->setItem(row, 5, new QTableWidgetItem("-"));
        auto* sItem = new QTableWidgetItem("Escalated");
        sItem->setForeground(QColor("#E85454"));
        sItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 6, sItem);
    }

    if (open > 0) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
        auto* catItem = new QTableWidgetItem("Complaint");
        catItem->setForeground(QColor("#FBBF24"));
        m_table->setItem(row, 1, catItem);
        auto* priItem = new QTableWidgetItem("Medium");
        priItem->setForeground(QColor("#FBBF24"));
        priItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 2, priItem);
        m_table->setItem(row, 3, new QTableWidgetItem(
            QString("%1 open complaints pending resolution").arg(open)));
        m_table->setItem(row, 4, new QTableWidgetItem("Assign and resolve open complaints"));
        m_table->setItem(row, 5, new QTableWidgetItem("-"));
        auto* sItem = new QTableWidgetItem("Active");
        sItem->setForeground(QColor("#FBBF24"));
        sItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 6, sItem);
    }
}

void AlertWidget::scanIncidentAlerts()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute("SELECT COUNT(*) FROM Incidents WHERE status = 'Open' OR status = 'Under Investigation'");
    int active = 0;
    if (query.next()) active = query.value(0).toInt();

    if (active == 0) return;

    int row = m_table->rowCount();
    m_table->insertRow(row);
    m_table->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
    auto* catItem = new QTableWidgetItem("Incident");
    catItem->setForeground(QColor("#E85454"));
    m_table->setItem(row, 1, catItem);
    auto* priItem = new QTableWidgetItem("High");
    priItem->setForeground(QColor("#FB923C"));
    priItem->setTextAlignment(Qt::AlignCenter);
    m_table->setItem(row, 2, priItem);
    m_table->setItem(row, 3, new QTableWidgetItem(
        QString("%1 incidents are open or under investigation").arg(active)));
    m_table->setItem(row, 4, new QTableWidgetItem("Review and resolve active incidents"));
    m_table->setItem(row, 5, new QTableWidgetItem("-"));
    auto* sItem = new QTableWidgetItem("Active");
    sItem->setForeground(QColor("#FB923C"));
    sItem->setTextAlignment(Qt::AlignCenter);
    m_table->setItem(row, 6, sItem);
}

void AlertWidget::refresh() { loadAlerts(); }

void AlertWidget::acknowledgeAlert()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select an alert to acknowledge."); return; }
    int row = items.first()->row();
    auto* statusItem = m_table->item(row, 6);
    if (statusItem) {
        statusItem->setText("Acknowledged");
        statusItem->setForeground(QColor("#60A5FA"));
    }
}

void AlertWidget::resolveAlert()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select an alert to dismiss."); return; }
    int row = items.first()->row();
    auto* statusItem = m_table->item(row, 6);
    if (statusItem) {
        statusItem->setText("Dismissed");
        statusItem->setForeground(QColor("#6B7585"));
    }
}

void AlertWidget::filterAlerts(const QString& text)
{
    QString searchText = text.toLower();
    QString catFilter = m_categoryFilter->currentText();
    QString priFilter = m_priorityFilter->currentText();
    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool textMatch = searchText.isEmpty();
        bool catMatch = (catFilter == "All");
        bool priMatch = (priFilter == "All");
        if (!textMatch) { for (int col = 1; col <= 5; ++col) { auto* item = m_table->item(row, col); if (item && item->text().toLower().contains(searchText)) { textMatch = true; break; } } }
        if (!catMatch) { auto* item = m_table->item(row, 1); if (item) catMatch = (item->text() == catFilter); }
        if (!priMatch) { auto* item = m_table->item(row, 2); if (item) priMatch = (item->text() == priFilter); }
        m_table->setRowHidden(row, !(textMatch && catMatch && priMatch));
    }
}

void AlertWidget::exportCSV()
{
    if (m_table->rowCount() == 0) { QMessageBox::information(this, "No Data", "No alerts to export."); return; }
    QDir().mkpath(QCoreApplication::applicationDirPath() + "/reports");
    QString filePath = QFileDialog::getSaveFileName(this, "Export Alerts", QCoreApplication::applicationDirPath() + "/reports/alerts.csv", "CSV Files (*.csv);;All Files (*)");
    if (filePath.isEmpty()) return;
    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text)) { QMessageBox::warning(this, "Error", "Could not open file."); return; }
    QTextStream out(&file);
    QStringList headers; QList<int> visibleCols;
    for (int col = 0; col < m_table->columnCount(); ++col) { visibleCols << col; auto* h = m_table->horizontalHeaderItem(col); headers << (h ? h->text() : ""); }
    out << headers.join(",") << "\n";
    for (int row = 0; row < m_table->rowCount(); ++row) {
        if (m_table->isRowHidden(row)) continue;
        QStringList rowParts;
        for (int col : visibleCols) { auto* item = m_table->item(row, col); QString text = item ? item->text() : ""; if (text.contains(',') || text.contains('"')) text = "\"" + text.replace("\"", "\"\"") + "\""; rowParts << text; }
        out << rowParts.join(",") << "\n";
    }
    file.close();
    QMessageBox::information(this, "Export Successful", QString("Alerts exported to:\n\n%1").arg(filePath));
}
