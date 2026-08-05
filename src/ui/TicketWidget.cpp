#include "TicketWidget.h"
#include "TicketDialog.h"
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

TicketWidget::TicketWidget(QWidget* parent) : QWidget(parent) { buildUI(); loadTickets(); }

void TicketWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Helpdesk / Tickets");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);
    auto* subtitle = new QLabel("Internal ticket system for IT and admin issues");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(8);

    auto* addBtn = new QPushButton("+ New Ticket");
    addBtn->setObjectName("PrimaryButton");
    addBtn->setFixedSize(130, 36);
    addBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &TicketWidget::addTicket);
    headerRow->addWidget(addBtn);

    auto* editBtn = new QPushButton("Edit");
    editBtn->setObjectName("SecondaryButton");
    editBtn->setFixedSize(70, 36);
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, &TicketWidget::editTicket);
    headerRow->addWidget(editBtn);

    auto* resolveBtn = new QPushButton("Resolve");
    resolveBtn->setFixedSize(90, 36);
    resolveBtn->setCursor(Qt::PointingHandCursor);
    resolveBtn->setStyleSheet("QPushButton { background-color: #1A3A1A; color: #4ADE80; border: 1px solid #2A4A2A; border-radius: 6px; padding: 6px 16px; font-weight: 600; } QPushButton:hover { background-color: #2A4A2A; }");
    connect(resolveBtn, &QPushButton::clicked, this, &TicketWidget::resolveTicket);
    headerRow->addWidget(resolveBtn);

    auto* delBtn = new QPushButton("Delete");
    delBtn->setObjectName("DangerButton");
    delBtn->setFixedSize(80, 36);
    delBtn->setCursor(Qt::PointingHandCursor);
    connect(delBtn, &QPushButton::clicked, this, &TicketWidget::deleteTicket);
    headerRow->addWidget(delBtn);

    auto* exportBtn = new QPushButton("Export CSV");
    exportBtn->setObjectName("SecondaryButton");
    exportBtn->setFixedSize(110, 36);
    exportBtn->setCursor(Qt::PointingHandCursor);
    connect(exportBtn, &QPushButton::clicked, this, &TicketWidget::exportCSV);
    headerRow->addWidget(exportBtn);

    headerRow->addStretch();
    m_countLabel = new QLabel;
    m_countLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    headerRow->addWidget(m_countLabel);
    mainLayout->addLayout(headerRow);

    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by code, subject, raised by...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &TicketWidget::filterTickets);
    filterRow->addWidget(m_searchEdit, 1);

    m_categoryFilter = new QComboBox;
    m_categoryFilter->addItems({"All", "IT Issue", "Hardware", "Software", "Network",
                                "Salary Issue", "Leave Issue", "Uniform Issue",
                                "Equipment Issue", "Complaint", "Suggestion", "General", "Other"});
    m_categoryFilter->setFixedWidth(140);
    connect(m_categoryFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterTickets(m_searchEdit->text()); });
    filterRow->addWidget(m_categoryFilter);

    m_priorityFilter = new QComboBox;
    m_priorityFilter->addItems({"All", "Low", "Medium", "High", "Critical"});
    m_priorityFilter->setFixedWidth(110);
    connect(m_priorityFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterTickets(m_searchEdit->text()); });
    filterRow->addWidget(m_priorityFilter);

    m_statusFilter = new QComboBox;
    m_statusFilter->addItems({"All", "Open", "In Progress", "Resolved", "Closed", "On Hold"});
    m_statusFilter->setFixedWidth(130);
    connect(m_statusFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterTickets(m_searchEdit->text()); });
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

    QStringList cols = {"ID", "Code", "Category", "Priority", "Subject",
                        "Raised By", "Assigned To", "Status", "Created"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);
    m_table->setColumnWidth(1, 90);
    m_table->setColumnWidth(2, 120);
    m_table->setColumnWidth(3, 80);
    m_table->setColumnWidth(4, 250);
    m_table->setColumnWidth(5, 120);
    m_table->setColumnWidth(6, 120);
    m_table->setColumnWidth(7, 110);
    m_table->horizontalHeader()->setStretchLastSection(true);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &TicketWidget::editTicket);
    mainLayout->addWidget(m_table, 1);
}

void TicketWidget::loadTickets()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute("SELECT * FROM Tickets ORDER BY created_at DESC");

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM Tickets");
    if (cc.next()) count = cc.value(0).toInt();
    m_table->setRowCount(count);
    m_table->setSortingEnabled(false);

    int row = 0, open = 0, inProgress = 0, resolved = 0, critical = 0;

    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        setItem(1, query.value("ticket_code").toString());
        setItem(2, query.value("category").toString());

        QString priority = query.value("priority").toString();
        auto* priItem = new QTableWidgetItem(priority);
        priItem->setTextAlignment(Qt::AlignCenter);
        if (priority == "Critical") { priItem->setForeground(QColor("#E85454")); critical++; }
        else if (priority == "High") priItem->setForeground(QColor("#FB923C"));
        else if (priority == "Medium") priItem->setForeground(QColor("#FBBF24"));
        else priItem->setForeground(QColor("#60A5FA"));
        m_table->setItem(row, 3, priItem);

        setItem(4, query.value("subject").toString());
        setItem(5, query.value("raised_by").toString());
        setItem(6, query.value("assigned_to").toString());

        QString status = query.value("status").toString();
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "Open") { statusItem->setForeground(QColor("#FBBF24")); open++; }
        else if (status == "In Progress") { statusItem->setForeground(QColor("#60A5FA")); inProgress++; }
        else if (status == "Resolved") { statusItem->setForeground(QColor("#4ADE80")); resolved++; }
        else if (status == "Closed") statusItem->setForeground(QColor("#6B7585"));
        else if (status == "On Hold") statusItem->setForeground(QColor("#A78BFA"));
        m_table->setItem(row, 7, statusItem);

        setItem(8, query.value("created_at").toString());
        row++;
    }
    m_table->setSortingEnabled(true);
    m_countLabel->setText(QString("%1 tickets").arg(row));
    m_summaryLabel->setText(
        QString("Total: %1 | Open: %2 | In Progress: %3 | Resolved: %4 | Critical: %5")
            .arg(row).arg(open).arg(inProgress).arg(resolved).arg(critical));
}

void TicketWidget::refresh() { loadTickets(); }

void TicketWidget::addTicket()
{
    TicketDialog dlg(this, -1);
    if (dlg.exec() == QDialog::Accepted) loadTickets();
}

void TicketWidget::editTicket()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a ticket to edit."); return; }
    int id = m_table->item(items.first()->row(), 0)->text().toInt();
    TicketDialog dlg(this, id);
    if (dlg.exec() == QDialog::Accepted) loadTickets();
}

void TicketWidget::resolveTicket()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a ticket to resolve."); return; }
    int row = items.first()->row();
    int id = m_table->item(row, 0)->text().toInt();
    QString code = m_table->item(row, 1)->text();
    QString status = m_table->item(row, 7)->text();
    if (status == "Resolved" || status == "Closed") { QMessageBox::information(this, "Already Resolved", "This ticket is already resolved."); return; }

    bool ok;
    QString resolution = QInputDialog::getText(this, "Resolve Ticket",
        QString("Enter resolution for %1:").arg(code), QLineEdit::Normal, "", &ok);
    if (ok && !resolution.trimmed().isEmpty()) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("UPDATE Tickets SET status = 'Resolved', resolution = :res, resolved_date = datetime('now','localtime') WHERE id = :id",
            {{":res", resolution.trimmed()}, {":id", id}});
        loadTickets();
    }
}

void TicketWidget::deleteTicket()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a ticket to delete."); return; }
    int row = items.first()->row();
    int id = m_table->item(row, 0)->text().toInt();
    QString code = m_table->item(row, 1)->text();
    auto result = QMessageBox::question(this, "Delete",
        QString("Delete ticket \"%1\"?").arg(code),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("DELETE FROM Tickets WHERE id = :id", {{":id", id}});
        loadTickets();
    }
}

void TicketWidget::filterTickets(const QString& text)
{
    QString searchText = text.toLower();
    QString catFilter = m_categoryFilter->currentText();
    QString priFilter = m_priorityFilter->currentText();
    QString statusFilter = m_statusFilter->currentText();
    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool textMatch = searchText.isEmpty();
        bool catMatch = (catFilter == "All");
        bool priMatch = (priFilter == "All");
        bool statusMatch = (statusFilter == "All");
        if (!textMatch) { for (int col = 1; col <= 6; ++col) { auto* item = m_table->item(row, col); if (item && item->text().toLower().contains(searchText)) { textMatch = true; break; } } }
        if (!catMatch) { auto* item = m_table->item(row, 2); if (item) catMatch = (item->text() == catFilter); }
        if (!priMatch) { auto* item = m_table->item(row, 3); if (item) priMatch = (item->text() == priFilter); }
        if (!statusMatch) { auto* item = m_table->item(row, 7); if (item) statusMatch = (item->text() == statusFilter); }
        m_table->setRowHidden(row, !(textMatch && catMatch && priMatch && statusMatch));
    }
}

void TicketWidget::exportCSV()
{
    if (m_table->rowCount() == 0) { QMessageBox::information(this, "No Data", "No tickets to export."); return; }
    QDir().mkpath(QCoreApplication::applicationDirPath() + "/reports");
    QString filePath = QFileDialog::getSaveFileName(this, "Export Tickets", QCoreApplication::applicationDirPath() + "/reports/tickets.csv", "CSV Files (*.csv);;All Files (*)");
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
    QMessageBox::information(this, "Export Successful", QString("Tickets exported to:\n\n%1").arg(filePath));
}
