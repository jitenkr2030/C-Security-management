#include "PayrollWidget.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDate>
#include <QCoreApplication>
#include <QDir>

PayrollWidget::PayrollWidget(QWidget* parent) : QWidget(parent) { buildUI(); loadPayroll(); }

void PayrollWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Payroll Generation");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);
    auto* subtitle = new QLabel("Generate monthly payroll for all guards with payslip export");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    // Controls
    auto* controlRow = new QHBoxLayout;
    controlRow->setSpacing(10);

    auto* monthLbl = new QLabel("Month:");
    monthLbl->setStyleSheet("color: #8B95A5; font-weight: 600;");
    controlRow->addWidget(monthLbl);

    m_monthCombo = new QComboBox;
    m_monthCombo->addItems({"January", "February", "March", "April", "May", "June",
                            "July", "August", "September", "October", "November", "December"});
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

    m_generateBtn = new QPushButton("Generate Payroll");
    m_generateBtn->setObjectName("PrimaryButton");
    m_generateBtn->setFixedSize(160, 36);
    m_generateBtn->setCursor(Qt::PointingHandCursor);
    connect(m_generateBtn, &QPushButton::clicked, this, &PayrollWidget::generatePayroll);
    controlRow->addWidget(m_generateBtn);

    auto* paidBtn = new QPushButton("Mark All Paid");
    paidBtn->setFixedSize(120, 36);
    paidBtn->setCursor(Qt::PointingHandCursor);
    paidBtn->setStyleSheet("QPushButton { background-color: #1A3A1A; color: #4ADE80; border: 1px solid #2A4A2A; border-radius: 6px; padding: 6px 16px; font-weight: 600; } QPushButton:hover { background-color: #2A4A2A; }");
    connect(paidBtn, &QPushButton::clicked, this, &PayrollWidget::markAllPaid);
    controlRow->addWidget(paidBtn);

    auto* payslipBtn = new QPushButton("Export Payslips");
    payslipBtn->setObjectName("SecondaryButton");
    payslipBtn->setFixedSize(140, 36);
    payslipBtn->setCursor(Qt::PointingHandCursor);
    connect(payslipBtn, &QPushButton::clicked, this, &PayrollWidget::exportPayslips);
    controlRow->addWidget(payslipBtn);

    auto* exportBtn = new QPushButton("Export CSV");
    exportBtn->setObjectName("SecondaryButton");
    exportBtn->setFixedSize(110, 36);
    exportBtn->setCursor(Qt::PointingHandCursor);
    connect(exportBtn, &QPushButton::clicked, this, &PayrollWidget::exportCSV);
    controlRow->addWidget(exportBtn);

    controlRow->addStretch();

    auto* loadBtn = new QPushButton("Load");
    loadBtn->setObjectName("SecondaryButton");
    loadBtn->setFixedSize(80, 36);
    loadBtn->setCursor(Qt::PointingHandCursor);
    connect(loadBtn, &QPushButton::clicked, this, &PayrollWidget::loadPayroll);
    controlRow->addWidget(loadBtn);

    mainLayout->addLayout(controlRow);

    // Summary
    m_summaryLabel = new QLabel;
    m_summaryLabel->setStyleSheet("color: #D4B44C; font-size: 13px; font-weight: 600;");
    mainLayout->addWidget(m_summaryLabel);

    m_countLabel = new QLabel;
    m_countLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    mainLayout->addWidget(m_countLabel);

    // Table
    m_table = new QTableWidget;
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(true);

    QStringList cols = {"ID", "Code", "Guard Name", "Basic", "HRA", "DA", "Overtime",
                        "Gross", "PF", "ESIC", "PT", "Fines", "Advance", "Deductions",
                        "Net Salary", "Status"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);
    m_table->setColumnWidth(1, 65);
    m_table->setColumnWidth(2, 140);
    m_table->setColumnWidth(3, 80);
    m_table->setColumnWidth(4, 70);
    m_table->setColumnWidth(5, 60);
    m_table->setColumnWidth(6, 75);
    m_table->setColumnWidth(7, 90);
    m_table->setColumnWidth(8, 70);
    m_table->setColumnWidth(9, 65);
    m_table->setColumnWidth(10, 55);
    m_table->setColumnWidth(11, 70);
    m_table->setColumnWidth(12, 70);
    m_table->setColumnWidth(13, 90);
    m_table->setColumnWidth(14, 100);
    m_table->horizontalHeader()->setStretchLastSection(true);
    mainLayout->addWidget(m_table, 1);
}

void PayrollWidget::loadPayroll()
{
    int month = m_monthCombo->currentIndex() + 1;
    int year = m_yearSpin->value();

    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT p.*, g.guard_code, g.full_name FROM Payroll p "
        "JOIN Guards g ON p.guard_id = g.id "
        "WHERE p.month = :m AND p.year = :y ORDER BY g.full_name",
        {{":m", month}, {":y", year}}
    );

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM Payroll WHERE month = :m AND year = :y",
                         {{":m", month}, {":y", year}});
    if (cc.next()) count = cc.value(0).toInt();
    m_table->setRowCount(count);
    m_table->setSortingEnabled(false);

    int row = 0;
    double totalGross = 0, totalNet = 0, totalDed = 0;
    int paid = 0, pending = 0;

    auto moneyItem = [](double val, const QColor& fg = QColor()) {
        auto* item = new QTableWidgetItem(QString::number(val, 'f', 0));
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (fg.isValid()) item->setForeground(QBrush(fg));
        return item;
    };

    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        setItem(1, query.value("guard_code").toString());
        setItem(2, query.value("full_name").toString());

        m_table->setItem(row, 3, moneyItem(query.value("basic_salary").toDouble()));
        m_table->setItem(row, 4, moneyItem(query.value("hra").toDouble()));
        m_table->setItem(row, 5, moneyItem(query.value("da").toDouble()));
        m_table->setItem(row, 6, moneyItem(query.value("overtime_pay").toDouble()));

        double gross = query.value("gross_salary").toDouble();
        totalGross += gross;
        m_table->setItem(row, 7, moneyItem(gross, QColor("#4ADE80")));

        m_table->setItem(row, 8, moneyItem(query.value("pf_deduction").toDouble()));
        m_table->setItem(row, 9, moneyItem(query.value("esic_deduction").toDouble()));
        m_table->setItem(row, 10, moneyItem(query.value("pt_deduction").toDouble()));
        m_table->setItem(row, 11, moneyItem(query.value("fines").toDouble(), QColor("#E85454")));
        m_table->setItem(row, 12, moneyItem(query.value("advance").toDouble(), QColor("#FB923C")));

        double ded = query.value("total_deduction").toDouble();
        totalDed += ded;
        m_table->setItem(row, 13, moneyItem(ded, QColor("#E85454")));

        double net = query.value("net_salary").toDouble();
        totalNet += net;
        m_table->setItem(row, 14, moneyItem(net, QColor("#D4B44C")));

        QString status = query.value("payment_status").toString();
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "Paid") { statusItem->setForeground(QColor("#4ADE80")); paid++; }
        else { statusItem->setForeground(QColor("#FBBF24")); pending++; }
        m_table->setItem(row, 15, statusItem);

        row++;
    }
    m_table->setSortingEnabled(true);
    m_countLabel->setText(QString("%1 employees in payroll").arg(row));
    m_summaryLabel->setText(
        QString("Month: %1/%2 | Gross: Rs. %3 | Deductions: Rs. %4 | Net: Rs. %5 | Paid: %6 | Pending: %7")
            .arg(m_monthCombo->currentText()).arg(year)
            .arg(totalGross, 0, 'f', 0).arg(totalDed, 0, 'f', 0).arg(totalNet, 0, 'f', 0)
            .arg(paid).arg(pending));
}

void PayrollWidget::generatePayroll()
{
    int month = m_monthCombo->currentIndex() + 1;
    int year = m_yearSpin->value();

    auto& db = DatabaseManager::instance();

    // Check if payroll already exists
    auto existing = db.execute("SELECT COUNT(*) FROM Payroll WHERE month = :m AND year = :y",
                               {{":m", month}, {":y", year}});
    if (existing.next() && existing.value(0).toInt() > 0) {
        auto result = QMessageBox::question(this, "Payroll Exists",
            QString("Payroll for %1 %2 already exists with %3 records.\n\nRegenerate? This will delete existing entries.")
                .arg(m_monthCombo->currentText()).arg(year).arg(existing.value(0).toInt()),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (result != QMessageBox::Yes) return;
        db.executeNonQuery("DELETE FROM Payroll WHERE month = :m AND year = :y",
                           {{":m", month}, {":y", year}});
    }

    // Get settings
    auto getSetting = [&](const QString& key, const QString& def) -> QString {
        auto q = db.execute("SELECT value FROM Settings WHERE key = :k", {{":k", key}});
        return q.next() ? q.value("value").toString() : def;
    };

    double pfRate = getSetting("pf_rate", "12").toDouble() / 100.0;
    double esicRate = getSetting("esic_rate", "0.75").toDouble() / 100.0;
    double ptAmount = getSetting("pt_deduction", "200").toDouble();
    double otRate = getSetting("overtime_rate", "1.5").toDouble();
    int workDays = getSetting("working_days", "26").toInt();

    // Get all active guards
    auto guards = db.execute("SELECT * FROM Guards WHERE status = 'Active' ORDER BY guard_code");
    int generated = 0;

    while (guards.next()) {
        int gid = guards.value("id").toInt();
        double basic = guards.value("basic_salary").toDouble();
        double dailyRate = basic / workDays;

        // Calculate HRA (10% of basic) and DA (5% of basic)
        double hra = basic * 0.10;
        double da = basic * 0.05;

        // Get overtime hours from attendance
        double otHours = 0;
        auto otQ = db.execute(
            "SELECT COUNT(*) FROM Attendance WHERE guard_id = :gid AND month(date) = :m "
            "AND year(date) = :y AND status = 'Night Shift'",
            {{":gid", gid}, {":m", month}, {":y", year}});
        if (otQ.next()) otHours = otQ.value(0).toInt() * 8.0;

        double otPay = otHours * (dailyRate / 8.0) * otRate;

        double gross = basic + hra + da + otPay;

        // Deductions
        double pf = basic * pfRate;
        double esic = gross * esicRate;
        double pt = ptAmount;

        // Get approved fines for this month
        double finesTotal = 0;
        auto finesQ = db.execute(
            "SELECT COALESCE(SUM(amount), 0) FROM Fines "
            "WHERE guard_id = :gid AND deduction_month = :m AND deduction_year = :y "
            "AND (status = 'Approved' OR status = 'Deducted')",
            {{":gid", gid}, {":m", month}, {":y", year}});
        if (finesQ.next()) finesTotal = finesQ.value(0).toDouble();

        double advance = 0;
        double penalty = 0;
        double otherDed = 0;
        double totalDed = pf + esic + pt + finesTotal + advance + penalty + otherDed;
        double net = gross - totalDed;

        db.executeNonQuery(
            "INSERT INTO Payroll (guard_id, month, year, basic_salary, hra, da, "
            "overtime_hours, overtime_pay, other_allowances, gross_salary, "
            "pf_deduction, esic_deduction, pt_deduction, fines, advance, penalty, "
            "other_deductions, total_deduction, net_salary, payment_status) "
            "VALUES (:gid, :m, :y, :bs, :hra, :da, :oth, :otp, 0, :gross, "
            ":pf, :esic, :pt, :fines, :adv, :pen, 0, :ded, :net, 'Pending')",
            {{":gid", gid}, {":m", month}, {":y", year},
             {":bs", basic}, {":hra", hra}, {":da", da},
             {":oth", otHours}, {":otp", otPay}, {":gross", gross},
             {":pf", pf}, {":esic", esic}, {":pt", pt},
             {":fines", finesTotal}, {":adv", advance}, {":pen", penalty},
             {":ded", totalDed}, {":net", net}});
        generated++;
    }

    QMessageBox::information(this, "Payroll Generated",
        QString("Payroll generated for %1 %2.\n\n%3 employees processed.")
            .arg(m_monthCombo->currentText()).arg(year).arg(generated));
    loadPayroll();
}

void PayrollWidget::markAllPaid()
{
    if (m_table->rowCount() == 0) { QMessageBox::information(this, "No Data", "Generate payroll first."); return; }
    int month = m_monthCombo->currentIndex() + 1;
    int year = m_yearSpin->value();

    auto result = QMessageBox::question(this, "Mark All Paid",
        QString("Mark all %1 payroll entries as Paid?\n\nThis cannot be undone.")
            .arg(m_table->rowCount()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery(
            "UPDATE Payroll SET payment_status = 'Paid', payment_date = date('now','localtime') "
            "WHERE month = :m AND year = :y AND payment_status = 'Pending'",
            {{":m", month}, {":y", year}});
        loadPayroll();
        QMessageBox::information(this, "Done", "All payroll entries marked as Paid.");
    }
}

void PayrollWidget::exportPayslips()
{
    if (m_table->rowCount() == 0) { QMessageBox::information(this, "No Data", "Generate payroll first."); return; }

    int month = m_monthCombo->currentIndex() + 1;
    int year = m_yearSpin->value();
    QString monthName = m_monthCombo->currentText();

    QDir().mkpath(QCoreApplication::applicationDirPath() + "/reports/payslips");
    QString dirPath = QCoreApplication::applicationDirPath() + "/reports/payslips";

    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT p.*, g.guard_code, g.full_name, g.father_name, g.mobile, "
        "g.aadhaar, g.pan FROM Payroll p "
        "JOIN Guards g ON p.guard_id = g.id "
        "WHERE p.month = :m AND p.year = :y ORDER BY g.guard_code",
        {{":m", month}, {":y", year}}
    );

    int count = 0;
    while (query.next()) {
        QString code = query.value("guard_code").toString();
        QString name = query.value("full_name").toString();

        QString payslip;
        payslip += "=====================================================\n";
        payslip += "              PAYSLIP / SALARY SLIP                  \n";
        payslip += "=====================================================\n";
        payslip += QString("Month: %1 %2\n").arg(monthName, QString::number(year));
        payslip += "-----------------------------------------------------\n";
        payslip += QString("Code:   %1\n").arg(code);
        payslip += QString("Name:   %1\n").arg(name);
        payslip += QString("Father: %1\n").arg(query.value("father_name").toString());
        payslip += QString("Mobile: %1\n").arg(query.value("mobile").toString());
        payslip += QString("PAN:    %1\n").arg(query.value("pan").toString());
        payslip += "-----------------------------------------------------\n";
        payslip += "EARNINGS                           AMOUNT (Rs.)\n";
        payslip += "-----------------------------------------------------\n";
        payslip += QString("Basic Salary                      %1\n").arg(query.value("basic_salary").toDouble(), 10, 'f', 0);
        payslip += QString("HRA (10%%)                          %1\n").arg(query.value("hra").toDouble(), 10, 'f', 0);
        payslip += QString("DA (5%%)                            %1\n").arg(query.value("da").toDouble(), 10, 'f', 0);
        payslip += QString("Overtime Pay                      %1\n").arg(query.value("overtime_pay").toDouble(), 10, 'f', 0);
        payslip += "-----------------------------------------------------\n";
        payslip += QString("GROSS SALARY                      %1\n").arg(query.value("gross_salary").toDouble(), 10, 'f', 0);
        payslip += "-----------------------------------------------------\n";
        payslip += "DEDUCTIONS                         AMOUNT (Rs.)\n";
        payslip += "-----------------------------------------------------\n";
        payslip += QString("PF Deduction                      %1\n").arg(query.value("pf_deduction").toDouble(), 10, 'f', 0);
        payslip += QString("ESIC Deduction                    %1\n").arg(query.value("esic_deduction").toDouble(), 10, 'f', 0);
        payslip += QString("Professional Tax                  %1\n").arg(query.value("pt_deduction").toDouble(), 10, 'f', 0);
        payslip += QString("Fines                             %1\n").arg(query.value("fines").toDouble(), 10, 'f', 0);
        payslip += QString("Advance                           %1\n").arg(query.value("advance").toDouble(), 10, 'f', 0);
        payslip += "-----------------------------------------------------\n";
        payslip += QString("TOTAL DEDUCTIONS                  %1\n").arg(query.value("total_deduction").toDouble(), 10, 'f', 0);
        payslip += "=====================================================\n";
        payslip += QString("NET SALARY                        %1\n").arg(query.value("net_salary").toDouble(), 10, 'f', 0);
        payslip += "=====================================================\n";
        payslip += QString("Payment Status: %1\n").arg(query.value("payment_status").toString());
        payslip += QString("Generated: %1\n").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"));

        QString fileName = dirPath + "/" + code + "_" + monthName + "_" + QString::number(year) + ".txt";
        QFile file(fileName);
        if (file.open(QFile::WriteOnly | QFile::Text)) {
            QTextStream out(&file);
            out << payslip;
            file.close();
            count++;
        }
    }

    QMessageBox::information(this, "Payslips Exported",
        QString("%1 payslips exported to:\n\n%2").arg(count).arg(dirPath));
}

void PayrollWidget::exportCSV()
{
    if (m_table->rowCount() == 0) { QMessageBox::information(this, "No Data", "No payroll to export."); return; }
    QDir().mkpath(QCoreApplication::applicationDirPath() + "/reports");
    QString defaultName = "payroll_" + m_monthCombo->currentText().toLower() + "_" + QString::number(m_yearSpin->value()) + ".csv";
    QString filePath = QFileDialog::getSaveFileName(this, "Export Payroll", QCoreApplication::applicationDirPath() + "/reports/" + defaultName, "CSV Files (*.csv);;All Files (*)");
    if (filePath.isEmpty()) return;
    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text)) { QMessageBox::warning(this, "Error", "Could not open file."); return; }
    QTextStream out(&file);
    QStringList headers; QList<int> visibleCols;
    for (int col = 0; col < m_table->columnCount(); ++col) { if (!m_table->isColumnHidden(col)) { visibleCols << col; auto* h = m_table->horizontalHeaderItem(col); headers << (h ? h->text() : ""); } }
    out << headers.join(",") << "\n";
    for (int row = 0; row < m_table->rowCount(); ++row) {
        QStringList rowParts;
        for (int col : visibleCols) { auto* item = m_table->item(row, col); QString text = item ? item->text() : ""; if (text.contains(',') || text.contains('"')) text = "\"" + text.replace("\"", "\"\"") + "\""; rowParts << text; }
        out << rowParts.join(",") << "\n";
    }
    file.close();
    QMessageBox::information(this, "Export Successful", QString("Payroll exported to:\n\n%1").arg(filePath));
}

void PayrollWidget::refresh() { loadPayroll(); }
