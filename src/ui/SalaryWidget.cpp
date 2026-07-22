#include "SalaryWidget.h"
#include "database/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDate>
#include <QCoreApplication>
#include <QDir>
#include <QFrame>

SalaryWidget::SalaryWidget(QWidget* parent)
    : QWidget(parent), m_pfRate(12), m_esicRate(0.75), m_ptDeduction(200),
      m_overtimeRate(1.5), m_workingDays(26)
{
    buildUI();
    loadSettings();
    loadSalaryData();
}

void SalaryWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Salary Management");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    auto* subtitle = new QLabel("Auto-calculate salary with PF, ESIC, overtime deductions");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    auto* tabWidget = new QTabWidget;
    tabWidget->addTab(buildPayrollTab(), "Monthly Payroll");
    tabWidget->addTab(buildSlipTab(), "Salary Slip");

    connect(tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 0) loadSalaryData();
    });

    mainLayout->addWidget(tabWidget, 1);
}

QWidget* SalaryWidget::buildPayrollTab()
{
    auto* tab = new QWidget;
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    // Controls
    auto* controlRow = new QHBoxLayout;
    controlRow->setSpacing(12);

    auto* monthLbl = new QLabel("Month:");
    monthLbl->setStyleSheet("color: #8B95A5; font-weight: 600;");
    controlRow->addWidget(monthLbl);

    m_monthCombo = new QComboBox;
    m_monthCombo->addItems({"January","February","March","April","May","June",
                            "July","August","September","October","November","December"});
    m_monthCombo->setCurrentIndex(QDate::currentDate().month() - 1);
    m_monthCombo->setFixedWidth(130);
    controlRow->addWidget(m_monthCombo);

    auto* yearLbl = new QLabel("Year:");
    yearLbl->setStyleSheet("color: #8B95A5; font-weight: 600;");
    controlRow->addWidget(yearLbl);

    m_yearSpin = new QSpinBox;
    m_yearSpin->setRange(2020, 2035);
    m_yearSpin->setValue(QDate::currentDate().year());
    m_yearSpin->setFixedWidth(90);
    controlRow->addWidget(m_yearSpin);
    controlRow->addSpacing(16);

    m_generateBtn = new QPushButton("Generate Salary");
    m_generateBtn->setObjectName("PrimaryButton");
    m_generateBtn->setFixedSize(160, 36);
    m_generateBtn->setCursor(Qt::PointingHandCursor);
    connect(m_generateBtn, &QPushButton::clicked, this, &SalaryWidget::generateSalary);
    controlRow->addWidget(m_generateBtn);

    controlRow->addStretch();
    layout->addLayout(controlRow);

    // Search row
    auto* searchRow = new QHBoxLayout;
    searchRow->setSpacing(10);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by name, code...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &SalaryWidget::filterSalary);
    searchRow->addWidget(m_searchEdit, 1);

    m_statusFilter = new QComboBox;
    m_statusFilter->addItems({"All Status", "Pending", "Paid", "Hold"});
    m_statusFilter->setFixedWidth(130);
    connect(m_statusFilter, &QComboBox::currentTextChanged, this,
            [this](const QString&) { filterSalary(m_searchEdit->text()); });
    searchRow->addWidget(m_statusFilter);

    layout->addLayout(searchRow);

    // Action buttons
    auto* actionRow = new QHBoxLayout;
    actionRow->setSpacing(8);

    m_payBtn = new QPushButton("Mark Paid");
    m_payBtn->setObjectName("SecondaryButton");
    m_payBtn->setFixedSize(110, 36);
    m_payBtn->setCursor(Qt::PointingHandCursor);
    connect(m_payBtn, &QPushButton::clicked, this, &SalaryWidget::markPaid);
    actionRow->addWidget(m_payBtn);

    m_slipBtn = new QPushButton("View Slip");
    m_slipBtn->setObjectName("SecondaryButton");
    m_slipBtn->setFixedSize(110, 36);
    m_slipBtn->setCursor(Qt::PointingHandCursor);
    connect(m_slipBtn, &QPushButton::clicked, this, &SalaryWidget::viewSlip);
    actionRow->addWidget(m_slipBtn);

    m_exportBtn = new QPushButton("Export CSV");
    m_exportBtn->setObjectName("SecondaryButton");
    m_exportBtn->setFixedSize(110, 36);
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    connect(m_exportBtn, &QPushButton::clicked, this, &SalaryWidget::exportPayroll);
    actionRow->addWidget(m_exportBtn);

    actionRow->addStretch();

    m_summaryLabel = new QLabel;
    m_summaryLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    actionRow->addWidget(m_summaryLabel);

    layout->addLayout(actionRow);

    // Table
    m_payrollTable = new QTableWidget;
    m_payrollTable->setAlternatingRowColors(true);
    m_payrollTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_payrollTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_payrollTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_payrollTable->verticalHeader()->setVisible(false);
    m_payrollTable->setShowGrid(false);
    m_payrollTable->setSortingEnabled(true);

    QStringList cols = {"ID", "Code", "Guard Name", "Basic", "HRA", "Convey",
                        "Medical", "Special", "Gross", "PF", "ESIC", "PT",
                        "Advance", "Penalty", "Deduction", "Net Salary", "Status"};
    m_payrollTable->setColumnCount(cols.size());
    m_payrollTable->setHorizontalHeaderLabels(cols);
    m_payrollTable->setColumnHidden(0, true);

    m_payrollTable->setColumnWidth(1, 80);
    m_payrollTable->setColumnWidth(2, 160);
    for (int c = 3; c <= 15; ++c) m_payrollTable->setColumnWidth(c, 85);
    m_payrollTable->setColumnWidth(16, 80);
    m_payrollTable->horizontalHeader()->setStretchLastSection(true);

    layout->addWidget(m_payrollTable, 1);
    return tab;
}

QWidget* SalaryWidget::buildSlipTab()
{
    auto* tab = new QWidget;
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    auto* controlRow = new QHBoxLayout;
    controlRow->setSpacing(12);

    auto* guardLbl = new QLabel("Guard:");
    guardLbl->setStyleSheet("color: #8B95A5; font-weight: 600;");
    controlRow->addWidget(guardLbl);

    m_slipGuardCombo = new QComboBox;
    m_slipGuardCombo->setFixedWidth(280);
    connect(m_slipGuardCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) {
        // Will reload slip when month/year changes
    });
    controlRow->addWidget(m_slipGuardCombo);

    auto* monthLbl = new QLabel("Month:");
    monthLbl->setStyleSheet("color: #8B95A5; font-weight: 600;");
    controlRow->addWidget(monthLbl);

    m_slipMonthCombo = new QComboBox;
    m_slipMonthCombo->addItems({"January","February","March","April","May","June",
                                "July","August","September","October","November","December"});
    m_slipMonthCombo->setCurrentIndex(QDate::currentDate().month() - 1);
    m_slipMonthCombo->setFixedWidth(130);
    controlRow->addWidget(m_slipMonthCombo);

    auto* yearLbl = new QLabel("Year:");
    yearLbl->setStyleSheet("color: #8B95A5; font-weight: 600;");
    controlRow->addWidget(yearLbl);

    m_slipYearSpin = new QSpinBox;
    m_slipYearSpin->setRange(2020, 2035);
    m_slipYearSpin->setValue(QDate::currentDate().year());
    m_slipYearSpin->setFixedWidth(90);
    controlRow->addWidget(m_slipYearSpin);

    auto* viewBtn = new QPushButton("View Slip");
    viewBtn->setObjectName("PrimaryButton");
    viewBtn->setFixedSize(110, 36);
    viewBtn->setCursor(Qt::PointingHandCursor);
    connect(viewBtn, &QPushButton::clicked, this, &SalaryWidget::viewSlip);
    controlRow->addWidget(viewBtn);

    controlRow->addStretch();
    layout->addLayout(controlRow);

    // Slip details table
    m_slipTable = new QTableWidget;
    m_slipTable->setAlternatingRowColors(true);
    m_slipTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_slipTable->verticalHeader()->setVisible(false);
    m_slipTable->setShowGrid(true);
    m_slipTable->setColumnCount(2);
    m_slipTable->setHorizontalHeaderLabels({"Component", "Amount (Rs.)"});
    m_slipTable->setColumnWidth(0, 250);
    m_slipTable->setColumnWidth(1, 150);
    m_slipTable->horizontalHeader()->setStretchLastSection(true);

    layout->addWidget(m_slipTable, 1);

    m_slipSummary = new QLabel;
    m_slipSummary->setStyleSheet("color: #D4B44C; font-size: 18px; font-weight: 700; padding: 12px;");
    layout->addWidget(m_slipSummary);

    return tab;
}

void SalaryWidget::loadSettings()
{
    auto& db = DatabaseManager::instance();
    auto getKey = [&](const QString& key, const QString& def) -> QString {
        auto q = db.execute("SELECT value FROM Settings WHERE key = :k", {{":k", key}});
        return q.next() ? q.value("value").toString() : def;
    };
    m_pfRate = getKey("pf_rate", "12").toDouble();
    m_esicRate = getKey("esic_rate", "0.75").toDouble();
    m_ptDeduction = getKey("pt_deduction", "200").toDouble();
    m_overtimeRate = getKey("overtime_rate", "1.5").toDouble();
    m_workingDays = getKey("working_days", "26").toInt();
}

void SalaryWidget::generateSalary()
{
    int month = m_monthCombo->currentIndex() + 1;
    int year = m_yearSpin->value();

    auto& db = DatabaseManager::instance();
    auto guards = db.execute(
        "SELECT id FROM Guards WHERE status = 'Active' ORDER BY full_name"
    );

    int count = 0;
    while (guards.next()) {
        calculateSalaryForGuard(guards.value("id").toInt(), month, year);
        count++;
    }

    loadSalaryData();

    QMessageBox::information(this, "Salary Generated",
        QString("Salary calculated for %1 active guards for %2 %3.")
            .arg(count).arg(m_monthCombo->currentText()).arg(year));
}

void SalaryWidget::calculateSalaryForGuard(int guardId, int month, int year)
{
    auto& db = DatabaseManager::instance();

    // Get guard salary info
    auto gq = db.execute(
        "SELECT basic_salary, hra, conveyance, medical_allowance, special_allowance "
        "FROM Guards WHERE id = :id", {{":id", guardId}}
    );
    if (!gq.next()) return;

    double basic = gq.value("basic_salary").toDouble();
    double hra = gq.value("hra").toDouble();
    double conv = gq.value("conveyance").toDouble();
    double med = gq.value("medical_allowance").toDouble();
    double spec = gq.value("special_allowance").toDouble();

    // Count attendance for the month
    QString startDate = QDate(year, month, 1).toString("yyyy-MM-dd");
    QString endDate = QDate(year, month, QDate(year, month, 1).daysInMonth()).toString("yyyy-MM-dd");

    auto aq = db.execute(
        "SELECT status, COUNT(*) AS cnt FROM Attendance "
        "WHERE guard_id = :gid AND date BETWEEN :start AND :end "
        "GROUP BY status",
        {{":gid", guardId}, {":start", startDate}, {":end", endDate}}
    );

    int present = 0, absent = 0, halfDay = 0, leave = 0, nightShift = 0;
    while (aq.next()) {
        QString s = aq.value("status").toString();
        int c = aq.value("cnt").toInt();
        if (s == "Present")          present += c;
        else if (s == "Absent")      absent += c;
        else if (s == "Half Day")    halfDay += c;
        else if (s == "Leave")       leave += c;
        else if (s == "Night Shift") nightShift += c;
    }

    int totalDays = QDate(year, month, 1).daysInMonth();
    int workedDays = present + nightShift + (halfDay / 2);

    // Pro-rata salary if not full month
    double perDay = (basic + hra + conv + med + spec) / m_workingDays;
    double workFactor = (double)workedDays / m_workingDays;
    if (workFactor > 1.0) workFactor = 1.0;

    double earnedBasic = basic * workFactor;
    double earnedHra = hra * workFactor;
    double earnedConv = conv * workFactor;
    double earnedMed = med * workFactor;
    double earnedSpec = spec * workFactor;
    double overtimePay = nightShift * (perDay * m_overtimeRate - perDay);
    if (overtimePay < 0) overtimePay = 0;

    double gross = earnedBasic + earnedHra + earnedConv + earnedMed + earnedSpec + overtimePay;

    // Deductions
    double pf = earnedBasic * m_pfRate / 100.0;
    double esic = gross * m_esicRate / 100.0;
    double pt = m_ptDeduction;

    // Get existing advance and penalty
    auto sq = db.execute(
        "SELECT advance, penalty FROM Salary "
        "WHERE guard_id = :gid AND month = :m AND year = :y",
        {{":gid", guardId}, {":m", month}, {":y", year}}
    );
    double advance = 0, penalty = 0;
    if (sq.next()) {
        advance = sq.value("advance").toDouble();
        penalty = sq.value("penalty").toDouble();
    }

    double totalDeduction = pf + esic + pt + advance + penalty;
    double net = gross - totalDeduction;
    if (net < 0) net = 0;

    // Insert or update salary record
    db.executeNonQuery(
        "INSERT INTO Salary ("
        "guard_id, month, year, working_days, present_days, absent_days, "
        "leave_days, overtime_hours, basic_salary, hra, conveyance, "
        "medical, special, overtime_pay, bonus, gross_salary, "
        "pf_deduction, esic_deduction, pt_deduction, advance, penalty, "
        "total_deduction, net_salary, payment_status"
        ") VALUES ("
        ":gid, :m, :y, :wd, :pd, :ad, :ld, :ot, :basic, :hra, :conv, "
        ":med, :spec, :otpay, 0, :gross, :pf, :esic, :pt, :adv, :pen, "
        ":ded, :net, 'Pending'"
        ") ON CONFLICT(guard_id, month, year) DO UPDATE SET "
        "working_days = excluded.working_days, "
        "present_days = excluded.present_days, "
        "absent_days = excluded.absent_days, "
        "leave_days = excluded.leave_days, "
        "overtime_hours = excluded.overtime_hours, "
        "basic_salary = excluded.basic_salary, "
        "hra = excluded.hra, conveyance = excluded.conveyance, "
        "medical = excluded.medical, special = excluded.special, "
        "overtime_pay = excluded.overtime_pay, gross_salary = excluded.gross_salary, "
        "pf_deduction = excluded.pf_deduction, esic_deduction = excluded.esic_deduction, "
        "pt_deduction = excluded.pt_deduction, "
        "total_deduction = excluded.total_deduction, "
        "net_salary = excluded.net_salary",
        {
            {":gid", guardId}, {":m", month}, {":y", year},
            {":wd", m_workingDays}, {":pd", workedDays}, {":ad", absent},
            {":ld", leave}, {":ot", nightShift},
            {":basic", earnedBasic}, {":hra", earnedHra}, {":conv", earnedConv},
            {":med", earnedMed}, {":spec", earnedSpec},
            {":otpay", overtimePay}, {":gross", gross},
            {":pf", pf}, {":esic", esic}, {":pt", pt},
            {":adv", advance}, {":pen", penalty},
            {":ded", totalDeduction}, {":net", net}
        }
    );
}

void SalaryWidget::loadSalaryData()
{
    auto& db = DatabaseManager::instance();

    // Populate guard combo for slip tab
    m_slipGuardCombo->blockSignals(true);
    int currentGuard = m_slipGuardCombo->currentData().toInt();
    m_slipGuardCombo->clear();
    m_slipGuardCombo->addItem("-- Select Guard --", 0);
    auto guards = db.execute("SELECT id, guard_code, full_name FROM Guards WHERE status = 'Active' ORDER BY full_name");
    while (guards.next()) {
        m_slipGuardCombo->addItem(
            guards.value("full_name").toString() + " (" + guards.value("guard_code").toString() + ")",
            guards.value("id").toInt()
        );
    }
    for (int i = 0; i < m_slipGuardCombo->count(); ++i) {
        if (m_slipGuardCombo->itemData(i).toInt() == currentGuard) {
            m_slipGuardCombo->setCurrentIndex(i); break;
        }
    }
    m_slipGuardCombo->blockSignals(false);

    int month = m_monthCombo->currentIndex() + 1;
    int year = m_yearSpin->value();

    // Load salary records
    auto query = db.execute(
        "SELECT s.*, g.guard_code, g.full_name "
        "FROM Salary s JOIN Guards g ON s.guard_id = g.id "
        "WHERE s.month = :m AND s.year = :y ORDER BY g.full_name",
        {{":m", month}, {":y", year}}
    );

    m_payrollTable->setSortingEnabled(false);

    int count = 0;
    auto cq = db.execute("SELECT COUNT(*) FROM Salary WHERE month = :m AND year = :y",
                          {{":m", month}, {":y", year}});
    if (cq.next()) count = cq.value(0).toInt();
    m_payrollTable->setRowCount(count);

    double totalGross = 0, totalNet = 0, totalDeduction = 0;
    int pending = 0, paid = 0;

    int row = 0;
    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_payrollTable->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        auto* codeItem = new QTableWidgetItem(query.value("guard_code").toString());
        codeItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_payrollTable->setItem(row, 1, codeItem);

        auto* nameItem = new QTableWidgetItem(query.value("full_name").toString());
        nameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_payrollTable->setItem(row, 2, nameItem);

        setItem(3, QString::number(query.value("basic_salary").toDouble(), 'f', 0));
        setItem(4, QString::number(query.value("hra").toDouble(), 'f', 0));
        setItem(5, QString::number(query.value("conveyance").toDouble(), 'f', 0));
        setItem(6, QString::number(query.value("medical").toDouble(), 'f', 0));
        setItem(7, QString::number(query.value("special").toDouble(), 'f', 0));
        setItem(8, QString::number(query.value("gross_salary").toDouble(), 'f', 0));
        setItem(9, QString::number(query.value("pf_deduction").toDouble(), 'f', 0));
        setItem(10, QString::number(query.value("esic_deduction").toDouble(), 'f', 0));
        setItem(11, QString::number(query.value("pt_deduction").toDouble(), 'f', 0));
        setItem(12, QString::number(query.value("advance").toDouble(), 'f', 0));
        setItem(13, QString::number(query.value("penalty").toDouble(), 'f', 0));
        setItem(14, QString::number(query.value("total_deduction").toDouble(), 'f', 0));

        double net = query.value("net_salary").toDouble();
        setItem(15, QString::number(net, 'f', 0));
        m_payrollTable->item(row, 15)->setForeground(QColor("#D4B44C"));

        QString status = query.value("payment_status").toString();
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "Paid") {
            statusItem->setForeground(QColor("#4ADE80"));
            paid++;
        } else if (status == "Pending") {
            statusItem->setForeground(QColor("#FBBF24"));
            pending++;
        } else {
            statusItem->setForeground(QColor("#E85454"));
        }
        m_payrollTable->setItem(row, 16, statusItem);

        totalGross += query.value("gross_salary").toDouble();
        totalNet += net;
        totalDeduction += query.value("total_deduction").toDouble();

        row++;
    }

    m_payrollTable->setSortingEnabled(true);

    m_summaryLabel->setText(
        QString("Gross: Rs. %1 | Deductions: Rs. %2 | Net: Rs. %3 | Pending: %4 | Paid: %5")
            .arg(totalGross, 0, 'f', 0)
            .arg(totalDeduction, 0, 'f', 0)
            .arg(totalNet, 0, 'f', 0)
            .arg(pending).arg(paid)
    );
}

void SalaryWidget::refresh()
{
    loadSettings();
    loadSalaryData();
}

void SalaryWidget::markPaid()
{
    auto items = m_payrollTable->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Select a salary record to mark as paid.");
        return;
    }

    int row = items.first()->row();
    int salaryId = m_payrollTable->item(row, 0)->text().toInt();
    QString guardName = m_payrollTable->item(row, 2)->text();
    QString currentStatus = m_payrollTable->item(row, 16)->text();

    if (currentStatus == "Paid") {
        QMessageBox::information(this, "Already Paid", "This salary is already marked as paid.");
        return;
    }

    auto result = QMessageBox::question(this, "Confirm Payment",
        QString("Mark salary as PAID for \"%1\"?").arg(guardName),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery(
            "UPDATE Salary SET payment_status = 'Paid', "
            "payment_date = date('now','localtime') WHERE id = :id",
            {{":id", salaryId}}
        );
        loadSalaryData();
    }
}

void SalaryWidget::viewSlip()
{
    auto items = m_payrollTable->selectedItems();
    int guardId = 0;
    int month = m_monthCombo->currentIndex() + 1;
    int year = m_yearSpin->value();

    if (!items.isEmpty()) {
        int row = items.first()->row();
        int salaryId = m_payrollTable->item(row, 0)->text().toInt();
        auto& db = DatabaseManager::instance();
        auto sq = db.execute("SELECT guard_id FROM Salary WHERE id = :id", {{":id", salaryId}});
        if (sq.next()) guardId = sq.value("guard_id").toInt();
    } else {
        guardId = m_slipGuardCombo->currentData().toInt();
        month = m_slipMonthCombo->currentIndex() + 1;
        year = m_slipYearSpin->value();
    }

    if (guardId == 0) {
        QMessageBox::information(this, "No Selection", "Select a guard to view salary slip.");
        return;
    }

    auto& db = DatabaseManager::instance();
    auto gq = db.execute("SELECT * FROM Guards WHERE id = :id", {{":id", guardId}});
    if (!gq.next()) return;

    auto sq = db.execute(
        "SELECT * FROM Salary WHERE guard_id = :gid AND month = :m AND year = :y",
        {{":gid", guardId}, {":m", month}, {":y", year}}
    );
    if (!sq.next()) {
        QMessageBox::information(this, "No Record",
            "No salary record found. Generate salary first.");
        return;
    }

    // Fill slip table
    m_slipTable->setRowCount(0);

    struct Row { QString label; QString value; bool isSection; bool isBold; };
    QList<Row> rows = {
        {"GUARD DETAILS", "", true, true},
        {"Guard Code", gq.value("guard_code").toString(), false, false},
        {"Name", gq.value("full_name").toString(), false, false},
        {"Working Days", QString::number(sq.value("working_days").toInt()), false, false},
        {"Present Days", QString::number(sq.value("present_days").toInt()), false, false},
        {"Absent Days", QString::number(sq.value("absent_days").toInt()), false, false},
        {"", "", false, false},
        {"EARNINGS", "", true, true},
        {"Basic Salary", "Rs. " + QString::number(sq.value("basic_salary").toDouble(), 'f', 0), false, false},
        {"HRA", "Rs. " + QString::number(sq.value("hra").toDouble(), 'f', 0), false, false},
        {"Conveyance", "Rs. " + QString::number(sq.value("conveyance").toDouble(), 'f', 0), false, false},
        {"Medical Allowance", "Rs. " + QString::number(sq.value("medical").toDouble(), 'f', 0), false, false},
        {"Special Allowance", "Rs. " + QString::number(sq.value("special").toDouble(), 'f', 0), false, false},
        {"Overtime Pay", "Rs. " + QString::number(sq.value("overtime_pay").toDouble(), 'f', 0), false, false},
        {"GROSS SALARY", "Rs. " + QString::number(sq.value("gross_salary").toDouble(), 'f', 0), false, true},
        {"", "", false, false},
        {"DEDUCTIONS", "", true, true},
        {"PF (" + QString::number(m_pfRate) + "%)", "Rs. " + QString::number(sq.value("pf_deduction").toDouble(), 'f', 0), false, false},
        {"ESIC (" + QString::number(m_esicRate) + "%)", "Rs. " + QString::number(sq.value("esic_deduction").toDouble(), 'f', 0), false, false},
        {"Professional Tax", "Rs. " + QString::number(sq.value("pt_deduction").toDouble(), 'f', 0), false, false},
        {"Advance", "Rs. " + QString::number(sq.value("advance").toDouble(), 'f', 0), false, false},
        {"Penalty", "Rs. " + QString::number(sq.value("penalty").toDouble(), 'f', 0), false, false},
        {"TOTAL DEDUCTION", "Rs. " + QString::number(sq.value("total_deduction").toDouble(), 'f', 0), false, true},
    };

    m_slipTable->setRowCount(rows.size());

    for (int r = 0; r < rows.size(); ++r) {
        const auto& row = rows[r];
        auto* labelItem = new QTableWidgetItem(row.label);
        auto* valueItem = new QTableWidgetItem(row.value);

        labelItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        if (row.isSection) {
            labelItem->setForeground(QBrush(QColor("#D4B44C")));
            QFont f = labelItem->font(); f.setBold(true); labelItem->setFont(f);
        }
        if (row.isBold) {
            QFont f1 = labelItem->font(); f1.setBold(true); labelItem->setFont(f1);
            QFont f2 = valueItem->font(); f2.setBold(true); valueItem->setFont(f2);
            valueItem->setForeground(QBrush(QColor("#D4B44C")));
        }

        m_slipTable->setItem(r, 0, labelItem);
        m_slipTable->setItem(r, 1, valueItem);
    }

    double net = sq.value("net_salary").toDouble();
    m_slipSummary->setText(QString("NET SALARY: Rs. %1").arg(net, 0, 'f', 0));
}

void SalaryWidget::exportPayroll()
{
    if (m_payrollTable->rowCount() == 0) {
        QMessageBox::information(this, "No Data", "No salary data to export.");
        return;
    }

    QString defaultName = QString("payroll_%1_%2.csv")
        .arg(m_monthCombo->currentText().toLower()).arg(m_yearSpin->value());

    QDir().mkpath(QCoreApplication::applicationDirPath() + "/reports");

    QString filePath = QFileDialog::getSaveFileName(
        this, "Export Payroll",
        QCoreApplication::applicationDirPath() + "/reports/" + defaultName,
        "CSV Files (*.csv);;All Files (*)"
    );
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file.");
        return;
    }

    QTextStream out(&file);

    QStringList headerParts;
    for (int col = 0; col < m_payrollTable->columnCount(); ++col) {
        auto* hItem = m_payrollTable->horizontalHeaderItem(col);
        headerParts << (hItem ? hItem->text() : "");
    }
    out << headerParts.join(",") << "\n";

    for (int row = 0; row < m_payrollTable->rowCount(); ++row) {
        QStringList rowParts;
        for (int col = 0; col < m_payrollTable->columnCount(); ++col) {
            auto* item = m_payrollTable->item(row, col);
            rowParts << (item ? item->text() : "");
        }
        out << rowParts.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, "Export Successful",
        QString("Payroll exported to:\n\n%1").arg(filePath));
}

void SalaryWidget::filterSalary(const QString& text)
{
    QString searchText = text.toLower();
    QString statusFilter = m_statusFilter->currentText();

    for (int row = 0; row < m_payrollTable->rowCount(); ++row) {
        bool textMatch = searchText.isEmpty();
        bool statusMatch = (statusFilter == "All Status");

        if (!textMatch) {
            for (int col = 1; col <= 2; ++col) {
                auto* item = m_payrollTable->item(row, col);
                if (item && item->text().toLower().contains(searchText)) {
                    textMatch = true; break;
                }
            }
        }

        if (!statusMatch) {
            auto* statusItem = m_payrollTable->item(row, 16);
            if (statusItem) statusMatch = (statusItem->text() == statusFilter);
        }

        m_payrollTable->setRowHidden(row, !(textMatch && statusMatch));
    }
}
