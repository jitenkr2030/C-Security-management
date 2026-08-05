#include "InvoiceWidget.h"
#include "InvoiceDialog.h"
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

InvoiceWidget::InvoiceWidget(QWidget* parent) : QWidget(parent) { buildUI(); loadInvoices(); }

void InvoiceWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Client Billing & Invoices");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);
    auto* subtitle = new QLabel("Generate monthly invoices for clients based on guards deployed");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(8);

    auto* addBtn = new QPushButton("+ Generate Invoice");
    addBtn->setObjectName("PrimaryButton");
    addBtn->setFixedSize(170, 36);
    addBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &InvoiceWidget::addInvoice);
    headerRow->addWidget(addBtn);

    auto* editBtn = new QPushButton("Edit");
    editBtn->setObjectName("SecondaryButton");
    editBtn->setFixedSize(70, 36);
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, &InvoiceWidget::editInvoice);
    headerRow->addWidget(editBtn);

    auto* paidBtn = new QPushButton("Mark Paid");
    paidBtn->setFixedSize(100, 36);
    paidBtn->setCursor(Qt::PointingHandCursor);
    paidBtn->setStyleSheet("QPushButton { background-color: #1A3A1A; color: #4ADE80; border: 1px solid #2A4A2A; border-radius: 6px; padding: 6px 16px; font-weight: 600; } QPushButton:hover { background-color: #2A4A2A; }");
    connect(paidBtn, &QPushButton::clicked, this, &InvoiceWidget::markPaid);
    headerRow->addWidget(paidBtn);

    auto* delBtn = new QPushButton("Delete");
    delBtn->setObjectName("DangerButton");
    delBtn->setFixedSize(80, 36);
    delBtn->setCursor(Qt::PointingHandCursor);
    connect(delBtn, &QPushButton::clicked, this, &InvoiceWidget::deleteInvoice);
    headerRow->addWidget(delBtn);

    auto* exportBtn = new QPushButton("Export CSV");
    exportBtn->setObjectName("SecondaryButton");
    exportBtn->setFixedSize(110, 36);
    exportBtn->setCursor(Qt::PointingHandCursor);
    connect(exportBtn, &QPushButton::clicked, this, &InvoiceWidget::exportCSV);
    headerRow->addWidget(exportBtn);

    headerRow->addStretch();
    m_countLabel = new QLabel;
    m_countLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    headerRow->addWidget(m_countLabel);
    mainLayout->addLayout(headerRow);

    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by invoice code, client...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &InvoiceWidget::filterInvoices);
    filterRow->addWidget(m_searchEdit, 1);

    m_clientFilter = new QComboBox;
    m_clientFilter->addItem("All Clients", 0);
    auto& db = DatabaseManager::instance();
    auto clients = db.execute("SELECT id, client_name FROM Clients ORDER BY client_name");
    while (clients.next()) m_clientFilter->addItem(clients.value("client_name").toString(), clients.value("id").toInt());
    m_clientFilter->setFixedWidth(180);
    connect(m_clientFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterInvoices(m_searchEdit->text()); });
    filterRow->addWidget(m_clientFilter);

    m_statusFilter = new QComboBox;
    m_statusFilter->addItems({"All", "Draft", "Sent", "Paid", "Overdue", "Cancelled"});
    m_statusFilter->setFixedWidth(110);
    connect(m_statusFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterInvoices(m_searchEdit->text()); });
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

    QStringList cols = {"ID", "Code", "Client", "Month", "Guards", "Working Days",
                        "Guard Charges", "GST", "Total", "Status", "Invoice Date", "Due Date"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);
    m_table->setColumnWidth(1, 110);
    m_table->setColumnWidth(2, 160);
    m_table->setColumnWidth(3, 100);
    m_table->setColumnWidth(4, 70);
    m_table->setColumnWidth(5, 90);
    m_table->setColumnWidth(6, 110);
    m_table->setColumnWidth(7, 90);
    m_table->setColumnWidth(8, 110);
    m_table->setColumnWidth(9, 90);
    m_table->setColumnWidth(10, 100);
    m_table->horizontalHeader()->setStretchLastSection(true);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &InvoiceWidget::editInvoice);
    mainLayout->addWidget(m_table, 1);
}

void InvoiceWidget::loadInvoices()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT inv.*, c.client_name FROM Invoices inv "
        "JOIN Clients c ON inv.client_id = c.id "
        "ORDER BY inv.invoice_date DESC"
    );

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM Invoices");
    if (cc.next()) count = cc.value(0).toInt();
    m_table->setRowCount(count);
    m_table->setSortingEnabled(false);

    int row = 0;
    double totalBilled = 0, totalPaid = 0, totalPending = 0;
    int paid = 0, pending = 0;

    QStringList monthNames = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                              "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        setItem(1, query.value("invoice_code").toString());
        setItem(2, query.value("client_name").toString());

        int month = query.value("invoice_month").toInt();
        int year = query.value("invoice_year").toInt();
        setItem(3, (month >= 1 && month <= 12 ? monthNames[month] : "") + " " + QString::number(year));

        auto* guardsItem = new QTableWidgetItem(query.value("guards_deployed").toString());
        guardsItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 4, guardsItem);

        auto* daysItem = new QTableWidgetItem(query.value("working_days").toString());
        daysItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 5, daysItem);

        double guardCharges = query.value("total_guard_charges").toDouble();
        auto* chargesItem = new QTableWidgetItem(QString("Rs. %1").arg(guardCharges, 0, 'f', 0));
        chargesItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(row, 6, chargesItem);

        double gst = query.value("gst_amount").toDouble();
        auto* gstItem = new QTableWidgetItem(QString("Rs. %1").arg(gst, 0, 'f', 0));
        gstItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(row, 7, gstItem);

        double total = query.value("total_amount").toDouble();
        totalBilled += total;
        auto* totalItem = new QTableWidgetItem(QString("Rs. %1").arg(total, 0, 'f', 0));
        totalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        totalItem->setForeground(QColor("#D4B44C"));
        totalItem->setFont(QFont("", -1, QFont::Bold));
        m_table->setItem(row, 8, totalItem);

        QString status = query.value("status").toString();
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "Paid") { statusItem->setForeground(QColor("#4ADE80")); paid++; totalPaid += total; }
        else if (status == "Sent") { statusItem->setForeground(QColor("#60A5FA")); pending++; totalPending += total; }
        else if (status == "Draft") { statusItem->setForeground(QColor("#FBBF24")); pending++; totalPending += total; }
        else if (status == "Overdue") { statusItem->setForeground(QColor("#E85454")); pending++; totalPending += total; }
        else if (status == "Cancelled") statusItem->setForeground(QColor("#6B7585"));
        m_table->setItem(row, 9, statusItem);

        setItem(10, query.value("invoice_date").toString());
        setItem(11, query.value("due_date").toString());
        row++;
    }
    m_table->setSortingEnabled(true);
    m_countLabel->setText(QString("%1 invoices").arg(row));
    m_summaryLabel->setText(
        QString("Total: %1 | Billed: Rs. %2 | Paid: Rs. %3 | Pending: Rs. %4")
            .arg(row).arg(totalBilled, 0, 'f', 0).arg(totalPaid, 0, 'f', 0).arg(totalPending, 0, 'f', 0));
}

void InvoiceWidget::refresh() { loadInvoices(); }

void InvoiceWidget::addInvoice()
{
    InvoiceDialog dlg(this, -1);
    if (dlg.exec() == QDialog::Accepted) loadInvoices();
}

void InvoiceWidget::editInvoice()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select an invoice to edit."); return; }
    int id = m_table->item(items.first()->row(), 0)->text().toInt();
    InvoiceDialog dlg(this, id);
    if (dlg.exec() == QDialog::Accepted) loadInvoices();
}

void InvoiceWidget::markPaid()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select an invoice to mark as paid."); return; }
    int row = items.first()->row();
    int id = m_table->item(row, 0)->text().toInt();
    QString code = m_table->item(row, 1)->text();
    QString status = m_table->item(row, 9)->text();
    if (status == "Paid") { QMessageBox::information(this, "Already Paid", "This invoice is already marked as paid."); return; }

    auto result = QMessageBox::question(this, "Mark Paid",
        QString("Mark invoice %1 as Paid?").arg(code),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("UPDATE Invoices SET status = 'Paid', payment_date = date('now','localtime') WHERE id = :id",
            {{":id", id}});
        loadInvoices();
    }
}

void InvoiceWidget::deleteInvoice()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select an invoice to delete."); return; }
    int row = items.first()->row();
    int id = m_table->item(row, 0)->text().toInt();
    QString code = m_table->item(row, 1)->text();
    auto result = QMessageBox::question(this, "Delete",
        QString("Delete invoice \"%1\"?").arg(code),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("DELETE FROM Invoices WHERE id = :id", {{":id", id}});
        loadInvoices();
    }
}

void InvoiceWidget::filterInvoices(const QString& text)
{
    QString searchText = text.toLower();
    int clientFilter = m_clientFilter->currentData().toInt();
    QString statusFilter = m_statusFilter->currentText();
    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool textMatch = searchText.isEmpty();
        bool clientMatch = (clientFilter == 0);
        bool statusMatch = (statusFilter == "All");
        if (!textMatch) { for (int col = 1; col <= 3; ++col) { auto* item = m_table->item(row, col); if (item && item->text().toLower().contains(searchText)) { textMatch = true; break; } } }
        if (!clientMatch) { auto* item = m_table->item(row, 2); if (item) clientMatch = item->text().contains(m_clientFilter->currentText()); }
        if (!statusMatch) { auto* item = m_table->item(row, 9); if (item) statusMatch = (item->text() == statusFilter); }
        m_table->setRowHidden(row, !(textMatch && clientMatch && statusMatch));
    }
}

void InvoiceWidget::exportCSV()
{
    if (m_table->rowCount() == 0) { QMessageBox::information(this, "No Data", "No invoices to export."); return; }
    QDir().mkpath(QCoreApplication::applicationDirPath() + "/reports");
    QString filePath = QFileDialog::getSaveFileName(this, "Export Invoices", QCoreApplication::applicationDirPath() + "/reports/invoices.csv", "CSV Files (*.csv);;All Files (*)");
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
    QMessageBox::information(this, "Export Successful", QString("Invoices exported to:\n\n%1").arg(filePath));
}
