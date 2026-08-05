#include "FineWidget.h"
#include "FineDialog.h"
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
#include <QInputDialog>

FineWidget::FineWidget(QWidget* parent) : QWidget(parent) { buildUI(); loadFines(); }

void FineWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Fines & Deductions Ledger");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);
    auto* subtitle = new QLabel("Track fines per guard with cumulative totals and salary deduction");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(8);

    auto* addBtn = new QPushButton("+ Record Fine");
    addBtn->setObjectName("PrimaryButton");
    addBtn->setFixedSize(140, 36);
    addBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &FineWidget::addFine);
    headerRow->addWidget(addBtn);

    auto* editBtn = new QPushButton("Edit");
    editBtn->setObjectName("SecondaryButton");
    editBtn->setFixedSize(70, 36);
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, &FineWidget::editFine);
    headerRow->addWidget(editBtn);

    auto* approveBtn = new QPushButton("Approve");
    approveBtn->setFixedSize(90, 36);
    approveBtn->setCursor(Qt::PointingHandCursor);
    approveBtn->setStyleSheet("QPushButton { background-color: #1A3A1A; color: #4ADE80; border: 1px solid #2A4A2A; border-radius: 6px; padding: 6px 16px; font-weight: 600; } QPushButton:hover { background-color: #2A4A2A; }");
    connect(approveBtn, &QPushButton::clicked, this, &FineWidget::approveFine);
    headerRow->addWidget(approveBtn);

    auto* summaryBtn = new QPushButton("Guard Summary");
    summaryBtn->setObjectName("SecondaryButton");
    summaryBtn->setFixedSize(130, 36);
    summaryBtn->setCursor(Qt::PointingHandCursor);
    connect(summaryBtn, &QPushButton::clicked, this, &FineWidget::showGuardSummary);
    headerRow->addWidget(summaryBtn);

    auto* delBtn = new QPushButton("Delete");
    delBtn->setObjectName("DangerButton");
    delBtn->setFixedSize(80, 36);
    delBtn->setCursor(Qt::PointingHandCursor);
    connect(delBtn, &QPushButton::clicked, this, &FineWidget::deleteFine);
    headerRow->addWidget(delBtn);

    auto* exportBtn = new QPushButton("Export CSV");
    exportBtn->setObjectName("SecondaryButton");
    exportBtn->setFixedSize(110, 36);
    exportBtn->setCursor(Qt::PointingHandCursor);
    connect(exportBtn, &QPushButton::clicked, this, &FineWidget::exportCSV);
    headerRow->addWidget(exportBtn);

    headerRow->addStretch();
    m_countLabel = new QLabel;
    m_countLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    headerRow->addWidget(m_countLabel);
    mainLayout->addLayout(headerRow);

    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by guard name, type, reason...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &FineWidget::filterFines);
    filterRow->addWidget(m_searchEdit, 1);

    m_typeFilter = new QComboBox;
    m_typeFilter->addItems({"All Types", "Late Arrival", "Early Departure", "Absent Without Leave",
                            "Uniform Violation", "Sleeping on Duty", "Misconduct",
                            "Damage to Property", "Mobile Phone Usage", "Negligence of Duty", "Other"});
    m_typeFilter->setFixedWidth(180);
    connect(m_typeFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterFines(m_searchEdit->text()); });
    filterRow->addWidget(m_typeFilter);

    m_statusFilter = new QComboBox;
    m_statusFilter->addItems({"All", "Pending", "Approved", "Deducted", "Cancelled"});
    m_statusFilter->setFixedWidth(120);
    connect(m_statusFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterFines(m_searchEdit->text()); });
    filterRow->addWidget(m_statusFilter);
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

    QStringList cols = {"ID", "Guard", "Code", "Fine Type", "Reason", "Amount",
                        "Fine Date", "Deduct Month", "Status", "Approved By"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);
    m_table->setColumnWidth(1, 150);
    m_table->setColumnWidth(2, 70);
    m_table->setColumnWidth(3, 150);
    m_table->setColumnWidth(4, 220);
    m_table->setColumnWidth(5, 90);
    m_table->setColumnWidth(6, 100);
    m_table->setColumnWidth(7, 110);
    m_table->setColumnWidth(8, 100);
    m_table->horizontalHeader()->setStretchLastSection(true);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &FineWidget::editFine);
    mainLayout->addWidget(m_table, 1);
}

void FineWidget::loadFines()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT f.*, g.guard_code, g.full_name FROM Fines f "
        "JOIN Guards g ON f.guard_id = g.id "
        "ORDER BY f.fine_date DESC"
    );

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM Fines");
    if (cc.next()) count = cc.value(0).toInt();
    m_table->setRowCount(count);
    m_table->setSortingEnabled(false);

    int row = 0;
    double totalAmount = 0, pendingAmt = 0, approvedAmt = 0;
    int pending = 0;

    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        setItem(1, query.value("full_name").toString());
        setItem(2, query.value("guard_code").toString());
        setItem(3, query.value("fine_type").toString());
        setItem(4, query.value("reason").toString());

        double amount = query.value("amount").toDouble();
        totalAmount += amount;
        auto* amtItem = new QTableWidgetItem(QString("Rs. %1").arg(amount, 0, 'f', 0));
        amtItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        amtItem->setForeground(QColor("#E85454"));
        m_table->setItem(row, 5, amtItem);

        setItem(6, query.value("fine_date").toString());

        int month = query.value("deduction_month").toInt();
        int year = query.value("deduction_year").toInt();
        QStringList months = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                              "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        setItem(7, (month >= 1 && month <= 12 ? months[month] : "") + " " + QString::number(year));

        QString status = query.value("status").toString();
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "Pending") { statusItem->setForeground(QColor("#FBBF24")); pending++; pendingAmt += amount; }
        else if (status == "Approved") { statusItem->setForeground(QColor("#60A5FA")); approvedAmt += amount; }
        else if (status == "Deducted") statusItem->setForeground(QColor("#4ADE80"));
        else if (status == "Cancelled") statusItem->setForeground(QColor("#6B7585"));
        m_table->setItem(row, 8, statusItem);

        setItem(9, query.value("approved_by").toString());
        row++;
    }
    m_table->setSortingEnabled(true);
    m_countLabel->setText(QString("%1 fines").arg(row));
    m_summaryLabel->setText(
        QString("Total Fines: %1 | Total Amount: Rs. %2 | Pending: %3 (Rs. %4) | Approved: Rs. %5")
            .arg(row).arg(totalAmount, 0, 'f', 0).arg(pending).arg(pendingAmt, 0, 'f', 0)
            .arg(approvedAmt, 0, 'f', 0));
}

void FineWidget::refresh() { loadFines(); }

void FineWidget::addFine()
{
    FineDialog dlg(this, -1);
    if (dlg.exec() == QDialog::Accepted) loadFines();
}

void FineWidget::editFine()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a fine to edit."); return; }
    int id = m_table->item(items.first()->row(), 0)->text().toInt();
    FineDialog dlg(this, id);
    if (dlg.exec() == QDialog::Accepted) loadFines();
}

void FineWidget::approveFine()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a fine to approve."); return; }
    int row = items.first()->row();
    int id = m_table->item(row, 0)->text().toInt();
    QString guard = m_table->item(row, 1)->text();
    QString status = m_table->item(row, 8)->text();
    if (status == "Approved" || status == "Deducted") { QMessageBox::information(this, "Already Approved", "This fine is already approved."); return; }
    if (status == "Cancelled") { QMessageBox::information(this, "Cancelled", "Cannot approve a cancelled fine."); return; }

    bool ok;
    QString approver = QInputDialog::getText(this, "Approve Fine",
        QString("Approve fine for %1?\nEnter approver name:").arg(guard),
        QLineEdit::Normal, "", &ok);
    if (ok && !approver.trimmed().isEmpty()) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("UPDATE Fines SET status = 'Approved', approved_by = :by WHERE id = :id",
            {{":by", approver.trimmed()}, {":id", id}});
        loadFines();
    }
}

void FineWidget::deleteFine()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a fine to delete."); return; }
    int row = items.first()->row();
    int id = m_table->item(row, 0)->text().toInt();
    QString guard = m_table->item(row, 1)->text();
    auto result = QMessageBox::question(this, "Delete",
        QString("Delete fine record for \"%1\"?").arg(guard),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("DELETE FROM Fines WHERE id = :id", {{":id", id}});
        loadFines();
    }
}

void FineWidget::filterFines(const QString& text)
{
    QString searchText = text.toLower();
    QString typeFilter = m_typeFilter->currentText();
    QString statusFilter = m_statusFilter->currentText();
    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool textMatch = searchText.isEmpty();
        bool typeMatch = (typeFilter == "All Types");
        bool statusMatch = (statusFilter == "All");
        if (!textMatch) { for (int col = 1; col <= 5; ++col) { auto* item = m_table->item(row, col); if (item && item->text().toLower().contains(searchText)) { textMatch = true; break; } } }
        if (!typeMatch) { auto* item = m_table->item(row, 3); if (item) typeMatch = (item->text() == typeFilter); }
        if (!statusMatch) { auto* item = m_table->item(row, 8); if (item) statusMatch = (item->text() == statusFilter); }
        m_table->setRowHidden(row, !(textMatch && typeMatch && statusMatch));
    }
}

void FineWidget::showGuardSummary()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT g.id, g.guard_code, g.full_name, "
        "COUNT(f.id) AS fine_count, "
        "COALESCE(SUM(f.amount), 0) AS total_fines, "
        "COALESCE(SUM(CASE WHEN f.status = 'Pending' THEN f.amount ELSE 0 END), 0) AS pending_amt, "
        "COALESCE(SUM(CASE WHEN f.status = 'Approved' THEN f.amount ELSE 0 END), 0) AS approved_amt "
        "FROM Guards g LEFT JOIN Fines f ON g.id = f.guard_id "
        "GROUP BY g.id HAVING fine_count > 0 "
        "ORDER BY total_fines DESC"
    );

    QString summary = "Guard-wise Fine Summary\n\n";
    summary += QString("%1 %2 %3 %4\n").arg("Guard", -25).arg("Code", -8).arg("Count", 8).arg("Total Fine", 12);
    summary += QString("-").repeated(60) + "\n";

    while (query.next()) {
        summary += QString("%1 %2 %3 %4\n")
            .arg(query.value("full_name").toString(), -25)
            .arg(query.value("guard_code").toString(), -8)
            .arg(query.value("fine_count").toString(), 8)
            .arg(QString("Rs. %1").arg(query.value("total_fines").toDouble(), 0, 'f', 0), 12);
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Guard-wise Fine Summary");
    msgBox.setText(summary);
    msgBox.setFont(QFont("Courier", 10));
    msgBox.exec();
}

void FineWidget::exportCSV()
{
    if (m_table->rowCount() == 0) { QMessageBox::information(this, "No Data", "No fines to export."); return; }
    QDir().mkpath(QCoreApplication::applicationDirPath() + "/reports");
    QString filePath = QFileDialog::getSaveFileName(this, "Export Fines", QCoreApplication::applicationDirPath() + "/reports/fines.csv", "CSV Files (*.csv);;All Files (*)");
    if (filePath.isEmpty()) return;
    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text)) { QMessageBox::warning(this, "Error", "Could not open file."); return; }
    QTextStream out(&file);
    QStringList headers; QList<int> visibleCols;
    for (int col = 0; col < m_table->columnCount(); ++col) { if (!m_table->isColumnHidden(col)) { visibleCols << col; auto* h = m_table->horizontalHeaderItem(col); headers << (h ? h->text() : ""); } }
    out << headers.join(",") << "\n";
    for (int row = 0; row < m_table->rowCount(); ++row) {
        if (m_table->isRowHidden(row)) continue;
        QStringList rowParts;
        for (int col : visibleCols) { auto* item = m_table->item(row, col); QString text = item ? item->text() : ""; if (text.contains(',') || text.contains('"')) text = "\"" + text.replace("\"", "\"\"") + "\""; rowParts << text; }
        out << rowParts.join(",") << "\n";
    }
    file.close();
    QMessageBox::information(this, "Export Successful", QString("Fines exported to:\n\n%1").arg(filePath));
}
