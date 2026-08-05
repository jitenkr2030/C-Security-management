#include "ReportsWidget.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDate>
#include <QCoreApplication>
#include <QDir>

ReportsWidget::ReportsWidget(QWidget* parent) : QWidget(parent)
{
    buildUI();
}

void ReportsWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Reports");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);
    auto* subtitle = new QLabel("Generate and export reports for all modules");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    // Controls
    auto* controlGroup = new QGroupBox("Report Parameters");
    auto* controlLayout = new QHBoxLayout(controlGroup);
    controlLayout->setSpacing(12);

    auto* typeLbl = new QLabel("Report Type:");
    typeLbl->setStyleSheet("color: #8B95A5; font-weight: 600;");
    controlLayout->addWidget(typeLbl);

    m_reportType = new QComboBox;
    m_reportType->addItems({"Guard List", "Attendance Summary", "Salary Report",
                            "Leave Report", "Visitor Log", "Incident Report",
                            "Uniform Inventory", "Equipment Inventory"});
    m_reportType->setFixedWidth(200);
    controlLayout->addWidget(m_reportType);

    auto* fromLbl = new QLabel("From:");
    fromLbl->setStyleSheet("color: #8B95A5; font-weight: 600;");
    controlLayout->addWidget(fromLbl);

    m_fromDate = new QDateEdit;
    m_fromDate->setCalendarPopup(true);
    m_fromDate->setDisplayFormat("yyyy-MM-dd");
    m_fromDate->setDate(QDate::currentDate().addMonths(-1));
    m_fromDate->setFixedWidth(130);
    controlLayout->addWidget(m_fromDate);

    auto* toLbl = new QLabel("To:");
    toLbl->setStyleSheet("color: #8B95A5; font-weight: 600;");
    controlLayout->addWidget(toLbl);

    m_toDate = new QDateEdit;
    m_toDate->setCalendarPopup(true);
    m_toDate->setDisplayFormat("yyyy-MM-dd");
    m_toDate->setDate(QDate::currentDate());
    m_toDate->setFixedWidth(130);
    controlLayout->addWidget(m_toDate);

    auto* siteLbl = new QLabel("Site:");
    siteLbl->setStyleSheet("color: #8B95A5; font-weight: 600;");
    controlLayout->addWidget(siteLbl);

    m_siteFilter = new QComboBox;
    m_siteFilter->addItem("All Sites", 0);
    auto& db = DatabaseManager::instance();
    auto sites = db.execute("SELECT id, site_name FROM Sites WHERE status = 'Active' ORDER BY site_name");
    while (sites.next()) {
        m_siteFilter->addItem(sites.value("site_name").toString(), sites.value("id").toInt());
    }
    m_siteFilter->setFixedWidth(150);
    controlLayout->addWidget(m_siteFilter);

    m_generateBtn = new QPushButton("Generate");
    m_generateBtn->setObjectName("PrimaryButton");
    m_generateBtn->setFixedSize(110, 36);
    m_generateBtn->setCursor(Qt::PointingHandCursor);
    connect(m_generateBtn, &QPushButton::clicked, this, &ReportsWidget::generateReport);
    controlLayout->addWidget(m_generateBtn);

    m_exportBtn = new QPushButton("Export CSV");
    m_exportBtn->setObjectName("SecondaryButton");
    m_exportBtn->setFixedSize(110, 36);
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    connect(m_exportBtn, &QPushButton::clicked, this, &ReportsWidget::exportReport);
    controlLayout->addWidget(m_exportBtn);

    controlLayout->addStretch();
    mainLayout->addWidget(controlGroup);

    // Report title
    m_titleLabel = new QLabel("Select a report type and click Generate");
    m_titleLabel->setStyleSheet("color: #D4B44C; font-size: 15px; font-weight: 700;");
    mainLayout->addWidget(m_titleLabel);

    // Summary
    m_summaryLabel = new QLabel;
    m_summaryLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    mainLayout->addWidget(m_summaryLabel);

    // Table
    m_table = new QTableWidget;
    m_table->setAlternatingRowColors(true);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(true);
    m_table->setColumnCount(0);
    m_table->setRowCount(0);
    mainLayout->addWidget(m_table, 1);
}

void ReportsWidget::refresh() {}

void ReportsWidget::generateReport()
{
    QString type = m_reportType->currentText();
    if (type == "Guard List")           generateGuardReport();
    else if (type == "Attendance Summary") generateAttendanceReport();
    else if (type == "Salary Report")   generateSalaryReport();
    else if (type == "Leave Report")    generateLeaveReport();
    else if (type == "Visitor Log")     generateVisitorReport();
    else if (type == "Incident Report") generateIncidentReport();
    else if (type == "Uniform Inventory") generateUniformReport();
    else if (type == "Equipment Inventory") generateEquipmentReport();
}

void ReportsWidget::generateGuardReport()
{
    m_titleLabel->setText("Guard List Report");

    auto& db = DatabaseManager::instance();
    QStringList cols = {"ID", "Code", "Name", "Mobile", "Aadhaar", "Site",
                        "Join Date", "Basic Salary", "Status"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);

    auto query = db.execute(
        "SELECT g.*, s.site_name FROM Guards g "
        "LEFT JOIN Sites s ON g.site_id = s.id "
        "ORDER BY g.full_name"
    );

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM Guards");
    if (cc.next()) count = cc.value(0).toInt();
    m_table->setRowCount(count);
    m_table->setSortingEnabled(false);

    int row = 0, active = 0, inactive = 0;
    double totalSalary = 0;

    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };
        setItem(0, query.value("id").toString());
        setItem(1, query.value("guard_code").toString());
        setItem(2, query.value("full_name").toString());
        setItem(3, query.value("mobile").toString());
        setItem(4, query.value("aadhaar").toString());
        setItem(5, query.value("site_name").toString());
        setItem(6, query.value("joining_date").toString());

        double salary = query.value("basic_salary").toDouble();
        totalSalary += salary;
        auto* salItem = new QTableWidgetItem(QString::number(salary, 'f', 0));
        salItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(row, 7, salItem);

        QString status = query.value("status").toString();
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "Active") { statusItem->setForeground(QColor("#4ADE80")); active++; }
        else { statusItem->setForeground(QColor("#E85454")); inactive++; }
        m_table->setItem(row, 8, statusItem);

        row++;
    }
    m_table->setSortingEnabled(true);
    m_table->setColumnWidth(1, 80);
    m_table->setColumnWidth(2, 160);
    m_table->setColumnWidth(5, 120);
    m_table->horizontalHeader()->setStretchLastSection(true);

    m_summaryLabel->setText(QString("Total: %1 | Active: %2 | Inactive: %3 | Total Basic Salary: Rs. %4")
        .arg(row).arg(active).arg(inactive).arg(totalSalary, 0, 'f', 0));
}

void ReportsWidget::generateAttendanceReport()
{
    m_titleLabel->setText("Attendance Summary Report");

    auto& db = DatabaseManager::instance();
    QString from = m_fromDate->date().toString("yyyy-MM-dd");
    QString to = m_toDate->date().toString("yyyy-MM-dd");
    int siteId = m_siteFilter->currentData().toInt();

    QStringList cols = {"ID", "Code", "Guard Name", "Present", "Absent", "Half Day",
                        "Leave", "Night Shift", "Total Days"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);

    QString whereClause = "WHERE g.status = 'Active'";
    QVariantMap params;
    params[":from"] = from;
    params[":to"] = to;

    if (siteId > 0) {
        whereClause += " AND g.site_id = :sid";
        params[":sid"] = siteId;
    }

    auto query = db.execute(
        "SELECT g.id, g.guard_code, g.full_name FROM Guards g " + whereClause + " ORDER BY g.full_name",
        params
    );

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM Guards " + whereClause, params);
    if (cc.next()) count = cc.value(0).toInt();
    m_table->setRowCount(count);
    m_table->setSortingEnabled(false);

    int row = 0;
    int totalPresent = 0, totalAbsent = 0;

    while (query.next()) {
        int gid = query.value("id").toInt();

        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            m_table->setItem(row, col, item);
        };

        auto* idItem = new QTableWidgetItem(QString::number(gid));
        m_table->setItem(row, 0, idItem);
        auto* codeItem = new QTableWidgetItem(query.value("guard_code").toString());
        codeItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_table->setItem(row, 1, codeItem);
        auto* nameItem = new QTableWidgetItem(query.value("full_name").toString());
        nameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_table->setItem(row, 2, nameItem);

        auto aq = db.execute(
            "SELECT status, COUNT(*) AS cnt FROM Attendance "
            "WHERE guard_id = :gid AND date BETWEEN :from AND :to GROUP BY status",
            {{":gid", gid}, {":from", from}, {":to", to}}
        );

        int present = 0, absent = 0, halfDay = 0, leave = 0, night = 0;
        while (aq.next()) {
            QString s = aq.value("status").toString();
            int c = aq.value("cnt").toInt();
            if (s == "Present") present += c;
            else if (s == "Absent") absent += c;
            else if (s == "Half Day") halfDay += c;
            else if (s == "Leave") leave += c;
            else if (s == "Night Shift") night += c;
        }

        totalPresent += present;
        totalAbsent += absent;

        setItem(3, QString::number(present));
        setItem(4, QString::number(absent));
        setItem(5, QString::number(halfDay));
        setItem(6, QString::number(leave));
        setItem(7, QString::number(night));
        setItem(8, QString::number(present + absent + halfDay + leave + night));

        row++;
    }
    m_table->setSortingEnabled(true);
    m_table->setColumnWidth(1, 80);
    m_table->setColumnWidth(2, 160);
    m_table->horizontalHeader()->setStretchLastSection(true);

    m_summaryLabel->setText(QString("Period: %1 to %2 | Guards: %3 | Total Present: %4 | Total Absent: %5")
        .arg(from).arg(to).arg(row).arg(totalPresent).arg(totalAbsent));
}

void ReportsWidget::generateSalaryReport()
{
    m_titleLabel->setText("Salary Report");

    auto& db = DatabaseManager::instance();
    int month = m_fromDate->date().month();
    int year = m_fromDate->date().year();

    QStringList cols = {"ID", "Code", "Guard Name", "Gross", "PF", "ESIC",
                        "PT", "Advance", "Penalty", "Deduction", "Net", "Status"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);

    auto query = db.execute(
        "SELECT s.*, g.guard_code, g.full_name FROM Salary s "
        "JOIN Guards g ON s.guard_id = g.id "
        "WHERE s.month = :m AND s.year = :y ORDER BY g.full_name",
        {{":m", month}, {":y", year}}
    );

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM Salary WHERE month = :m AND year = :y",
                         {{":m", month}, {":y", year}});
    if (cc.next()) count = cc.value(0).toInt();
    m_table->setRowCount(count);
    m_table->setSortingEnabled(false);

    int row = 0;
    double totalGross = 0, totalNet = 0, totalDed = 0;
    int pending = 0, paid = 0;

    while (query.next()) {
        auto setItem = [&](int col, const QString& text, const QColor& fg = QColor()) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            if (fg.isValid()) item->setForeground(QBrush(fg));
            m_table->setItem(row, col, item);
        };

        auto* idItem = new QTableWidgetItem(query.value("id").toString());
        m_table->setItem(row, 0, idItem);
        auto* codeItem = new QTableWidgetItem(query.value("guard_code").toString());
        codeItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_table->setItem(row, 1, codeItem);
        auto* nameItem = new QTableWidgetItem(query.value("full_name").toString());
        nameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_table->setItem(row, 2, nameItem);

        double gross = query.value("gross_salary").toDouble();
        double ded = query.value("total_deduction").toDouble();
        double net = query.value("net_salary").toDouble();
        totalGross += gross;
        totalDed += ded;
        totalNet += net;

        setItem(3, QString::number(gross, 'f', 0));
        setItem(4, QString::number(query.value("pf_deduction").toDouble(), 'f', 0));
        setItem(5, QString::number(query.value("esic_deduction").toDouble(), 'f', 0));
        setItem(6, QString::number(query.value("pt_deduction").toDouble(), 'f', 0));
        setItem(7, QString::number(query.value("advance").toDouble(), 'f', 0));
        setItem(8, QString::number(query.value("penalty").toDouble(), 'f', 0));
        setItem(9, QString::number(ded, 'f', 0));
        setItem(10, QString::number(net, 'f', 0), QColor("#D4B44C"));

        QString status = query.value("payment_status").toString();
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "Paid") { statusItem->setForeground(QColor("#4ADE80")); paid++; }
        else { statusItem->setForeground(QColor("#FBBF24")); pending++; }
        m_table->setItem(row, 11, statusItem);

        row++;
    }
    m_table->setSortingEnabled(true);
    m_table->horizontalHeader()->setStretchLastSection(true);

    m_summaryLabel->setText(QString("Month: %1/%2 | Guards: %3 | Gross: Rs. %4 | Deductions: Rs. %5 | Net: Rs. %6 | Pending: %7 | Paid: %8")
        .arg(month).arg(year).arg(row).arg(totalGross, 0, 'f', 0).arg(totalDed, 0, 'f', 0)
        .arg(totalNet, 0, 'f', 0).arg(pending).arg(paid));
}

void ReportsWidget::generateLeaveReport()
{
    m_titleLabel->setText("Leave Report");

    auto& db = DatabaseManager::instance();
    QString from = m_fromDate->date().toString("yyyy-MM-dd");
    QString to = m_toDate->date().toString("yyyy-MM-dd");

    QStringList cols = {"ID", "Code", "Guard Name", "Leave Type", "Start", "End", "Days", "Status"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);

    auto query = db.execute(
        "SELECT l.*, g.guard_code, g.full_name FROM LeaveRecord l "
        "JOIN Guards g ON l.guard_id = g.id "
        "WHERE l.start_date >= :from AND l.start_date <= :to "
        "ORDER BY l.start_date DESC",
        {{":from", from}, {":to", to}}
    );

    m_table->setRowCount(0);
    m_table->setSortingEnabled(false);
    int row = 0;

    while (query.next()) {
        m_table->insertRow(row);
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };
        setItem(0, query.value("id").toString());
        setItem(1, query.value("guard_code").toString());
        setItem(2, query.value("full_name").toString());
        setItem(3, query.value("leave_type").toString());
        setItem(4, query.value("start_date").toString());
        setItem(5, query.value("end_date").toString());

        auto* daysItem = new QTableWidgetItem(query.value("days").toString());
        daysItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 6, daysItem);

        QString status = query.value("status").toString();
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "Approved") statusItem->setForeground(QColor("#4ADE80"));
        else if (status == "Rejected") statusItem->setForeground(QColor("#E85454"));
        else if (status == "Pending") statusItem->setForeground(QColor("#FBBF24"));
        m_table->setItem(row, 7, statusItem);

        row++;
    }
    m_table->setSortingEnabled(true);
    m_table->horizontalHeader()->setStretchLastSection(true);

    m_summaryLabel->setText(QString("Period: %1 to %2 | Total Leave Requests: %3").arg(from).arg(to).arg(row));
}

void ReportsWidget::generateVisitorReport()
{
    m_titleLabel->setText("Visitor Log Report");

    auto& db = DatabaseManager::instance();
    QString from = m_fromDate->date().toString("yyyy-MM-dd");
    QString to = m_toDate->date().toString("yyyy-MM-dd");

    QStringList cols = {"ID", "Site", "Name", "Mobile", "Purpose", "To Meet", "Entry", "Exit", "Vehicle"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);

    auto query = db.execute(
        "SELECT v.*, s.site_name FROM Visitors v "
        "JOIN Sites s ON v.site_id = s.id "
        "WHERE DATE(v.entry_time) BETWEEN :from AND :to "
        "ORDER BY v.entry_time DESC",
        {{":from", from}, {":to", to}}
    );

    m_table->setRowCount(0);
    m_table->setSortingEnabled(false);
    int row = 0, inside = 0;

    while (query.next()) {
        m_table->insertRow(row);
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };
        setItem(0, query.value("id").toString());
        setItem(1, query.value("site_name").toString());
        setItem(2, query.value("name").toString());
        setItem(3, query.value("mobile").toString());
        setItem(4, query.value("purpose").toString());
        setItem(5, query.value("whom_to_meet").toString());
        setItem(6, query.value("entry_time").toString());

        QString exitTime = query.value("exit_time").toString();
        setItem(7, exitTime.isEmpty() ? "Still Inside" : exitTime);
        if (exitTime.isEmpty()) {
            m_table->item(row, 7)->setForeground(QColor("#FBBF24"));
            inside++;
        }
        setItem(8, query.value("vehicle_no").toString());

        row++;
    }
    m_table->setSortingEnabled(true);
    m_table->horizontalHeader()->setStretchLastSection(true);

    m_summaryLabel->setText(QString("Period: %1 to %2 | Total Visitors: %3 | Currently Inside: %4")
        .arg(from).arg(to).arg(row).arg(inside));
}

void ReportsWidget::generateIncidentReport()
{
    m_titleLabel->setText("Incident Report");

    auto& db = DatabaseManager::instance();
    QString from = m_fromDate->date().toString("yyyy-MM-dd");
    QString to = m_toDate->date().toString("yyyy-MM-dd");

    QStringList cols = {"ID", "Code", "Type", "Severity", "Site", "Date", "Description", "Status"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);

    auto query = db.execute(
        "SELECT i.*, s.site_name FROM Incidents i "
        "LEFT JOIN Sites s ON i.site_id = s.id "
        "WHERE DATE(i.date_time) BETWEEN :from AND :to "
        "ORDER BY i.date_time DESC",
        {{":from", from}, {":to", to}}
    );

    m_table->setRowCount(0);
    m_table->setSortingEnabled(false);
    int row = 0, open = 0, critical = 0;

    while (query.next()) {
        m_table->insertRow(row);
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };
        setItem(0, query.value("id").toString());
        setItem(1, query.value("incident_code").toString());
        setItem(2, query.value("incident_type").toString());

        QString sev = query.value("severity").toString();
        auto* sevItem = new QTableWidgetItem(sev);
        sevItem->setTextAlignment(Qt::AlignCenter);
        if (sev == "Critical") { sevItem->setForeground(QColor("#E85454")); critical++; }
        else if (sev == "High") sevItem->setForeground(QColor("#FB923C"));
        else if (sev == "Medium") sevItem->setForeground(QColor("#FBBF24"));
        m_table->setItem(row, 3, sevItem);

        setItem(4, query.value("site_name").toString());
        setItem(5, query.value("date_time").toString());
        setItem(6, query.value("description").toString());

        QString status = query.value("status").toString();
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "Open") { statusItem->setForeground(QColor("#FBBF24")); open++; }
        else if (status == "Resolved") statusItem->setForeground(QColor("#4ADE80"));
        m_table->setItem(row, 7, statusItem);

        row++;
    }
    m_table->setSortingEnabled(true);
    m_table->horizontalHeader()->setStretchLastSection(true);

    m_summaryLabel->setText(QString("Period: %1 to %2 | Total: %3 | Open: %4 | Critical: %5")
        .arg(from).arg(to).arg(row).arg(open).arg(critical));
}

void ReportsWidget::generateUniformReport()
{
    m_titleLabel->setText("Uniform Inventory Report");

    auto& db = DatabaseManager::instance();
    QStringList cols = {"ID", "Item Type", "Size", "Quantity", "Status", "Notes"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);

    auto query = db.execute("SELECT * FROM Uniform ORDER BY item_type, size");

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM Uniform");
    if (cc.next()) count = cc.value(0).toInt();
    m_table->setRowCount(count);
    m_table->setSortingEnabled(false);

    int row = 0, totalQty = 0;

    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };
        setItem(0, query.value("id").toString());
        setItem(1, query.value("item_type").toString());
        setItem(2, query.value("size").toString());

        int qty = query.value("quantity").toInt();
        totalQty += qty;
        auto* qtyItem = new QTableWidgetItem(QString::number(qty));
        qtyItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 3, qtyItem);

        setItem(4, query.value("status").toString());
        setItem(5, query.value("notes").toString());
        row++;
    }
    m_table->setSortingEnabled(true);
    m_table->horizontalHeader()->setStretchLastSection(true);

    m_summaryLabel->setText(QString("Total Items: %1 | Total Quantity: %2").arg(row).arg(totalQty));
}

void ReportsWidget::generateEquipmentReport()
{
    m_titleLabel->setText("Equipment Inventory Report");

    auto& db = DatabaseManager::instance();
    QStringList cols = {"ID", "Code", "Type", "Serial No.", "Condition", "Status", "Purchase Date"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);

    auto query = db.execute("SELECT * FROM Equipment ORDER BY equipment_code");

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM Equipment");
    if (cc.next()) count = cc.value(0).toInt();
    m_table->setRowCount(count);
    m_table->setSortingEnabled(false);

    int row = 0, avail = 0, issued = 0;

    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };
        setItem(0, query.value("id").toString());
        setItem(1, query.value("equipment_code").toString());
        setItem(2, query.value("equipment_type").toString());
        setItem(3, query.value("serial_number").toString());

        QString cond = query.value("condition").toString();
        auto* condItem = new QTableWidgetItem(cond);
        condItem->setTextAlignment(Qt::AlignCenter);
        if (cond == "New") condItem->setForeground(QColor("#4ADE80"));
        else if (cond == "Damaged") condItem->setForeground(QColor("#E85454"));
        m_table->setItem(row, 4, condItem);

        QString status = query.value("status").toString();
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "Available") { statusItem->setForeground(QColor("#4ADE80")); avail++; }
        else if (status == "Issued") { statusItem->setForeground(QColor("#FBBF24")); issued++; }
        m_table->setItem(row, 5, statusItem);

        setItem(6, query.value("purchase_date").toString());
        row++;
    }
    m_table->setSortingEnabled(true);
    m_table->horizontalHeader()->setStretchLastSection(true);

    m_summaryLabel->setText(QString("Total: %1 | Available: %2 | Issued: %3").arg(row).arg(avail).arg(issued));
}

void ReportsWidget::exportReport()
{
    if (m_table->rowCount() == 0) {
        QMessageBox::information(this, "No Data", "Generate a report first before exporting.");
        return;
    }

    QDir().mkpath(QCoreApplication::applicationDirPath() + "/reports");
    QString defaultName = m_reportType->currentText().toLower().replace(" ", "_") + "_report.csv";

    QString filePath = QFileDialog::getSaveFileName(
        this, "Export Report",
        QCoreApplication::applicationDirPath() + "/reports/" + defaultName,
        "CSV Files (*.csv);;All Files (*)");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file.");
        return;
    }

    QTextStream out(&file);
    QStringList headers;
    QList<int> visibleCols;
    for (int col = 0; col < m_table->columnCount(); ++col) {
        if (!m_table->isColumnHidden(col)) {
            visibleCols << col;
            auto* h = m_table->horizontalHeaderItem(col);
            headers << (h ? h->text() : "");
        }
    }
    out << headers.join(",") << "\n";

    for (int row = 0; row < m_table->rowCount(); ++row) {
        if (m_table->isRowHidden(row)) continue;
        QStringList rowParts;
        for (int col : visibleCols) {
            auto* item = m_table->item(row, col);
            QString text = item ? item->text() : "";
            if (text.contains(',') || text.contains('"'))
                text = "\"" + text.replace("\"", "\"\"") + "\"";
            rowParts << text;
        }
        out << rowParts.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, "Export Successful",
        QString("Report exported to:\n\n%1").arg(filePath));
}
