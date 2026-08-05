#include "AuditLogWidget.h"
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

AuditLogWidget::AuditLogWidget(QWidget* parent) : QWidget(parent) { buildUI(); loadLogs(); }

void AuditLogWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Audit Log");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);
    auto* subtitle = new QLabel("Track all create, edit and delete actions across the system");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(8);

    auto* refreshBtn = new QPushButton("Refresh");
    refreshBtn->setObjectName("PrimaryButton");
    refreshBtn->setFixedSize(100, 36);
    refreshBtn->setCursor(Qt::PointingHandCursor);
    connect(refreshBtn, &QPushButton::clicked, this, &AuditLogWidget::loadLogs);
    headerRow->addWidget(refreshBtn);

    auto* clearBtn = new QPushButton("Clear Old Logs");
    clearBtn->setObjectName("SecondaryButton");
    clearBtn->setFixedSize(140, 36);
    clearBtn->setCursor(Qt::PointingHandCursor);
    connect(clearBtn, &QPushButton::clicked, this, &AuditLogWidget::clearOldLogs);
    headerRow->addWidget(clearBtn);

    auto* exportBtn = new QPushButton("Export CSV");
    exportBtn->setObjectName("SecondaryButton");
    exportBtn->setFixedSize(110, 36);
    exportBtn->setCursor(Qt::PointingHandCursor);
    connect(exportBtn, &QPushButton::clicked, this, &AuditLogWidget::exportCSV);
    headerRow->addWidget(exportBtn);

    headerRow->addStretch();
    m_countLabel = new QLabel;
    m_countLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    headerRow->addWidget(m_countLabel);
    mainLayout->addLayout(headerRow);

    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by action, module, details...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &AuditLogWidget::filterLogs);
    filterRow->addWidget(m_searchEdit, 1);

    m_moduleFilter = new QComboBox;
    m_moduleFilter->addItems({"All", "Guards", "Clients", "Sites", "Attendance", "Duty",
                              "Leave", "Salary", "Uniform", "Equipment", "Visitors",
                              "Vehicles", "Incidents", "Training", "Documents",
                              "Complaints", "Fines", "Payroll", "Announcements",
                              "Photos", "Invoices", "Tickets", "Settings", "Backup"});
    m_moduleFilter->setFixedWidth(130);
    connect(m_moduleFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterLogs(m_searchEdit->text()); });
    filterRow->addWidget(m_moduleFilter);

    m_actionFilter = new QComboBox;
    m_actionFilter->addItems({"All", "CREATE", "UPDATE", "DELETE", "LOGIN", "EXPORT", "BACKUP", "RESTORE"});
    m_actionFilter->setFixedWidth(110);
    connect(m_actionFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterLogs(m_searchEdit->text()); });
    filterRow->addWidget(m_actionFilter);

    auto* fromLbl = new QLabel("From:");
    fromLbl->setStyleSheet("color: #8B95A5; font-weight: 600;");
    filterRow->addWidget(fromLbl);
    m_fromDate = new QDateEdit;
    m_fromDate->setCalendarPopup(true);
    m_fromDate->setDisplayFormat("yyyy-MM-dd");
    m_fromDate->setDate(QDate::currentDate().addMonths(-3));
    m_fromDate->setFixedWidth(120);
    connect(m_fromDate, &QDateEdit::dateChanged, this, [this]() { filterLogs(m_searchEdit->text()); });
    filterRow->addWidget(m_fromDate);
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

    QStringList cols = {"ID", "User", "Action", "Module", "Record ID", "Details", "Timestamp"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);
    m_table->setColumnWidth(1, 100);
    m_table->setColumnWidth(2, 80);
    m_table->setColumnWidth(3, 110);
    m_table->setColumnWidth(4, 80);
    m_table->setColumnWidth(5, 350);
    m_table->horizontalHeader()->setStretchLastSection(true);
    mainLayout->addWidget(m_table, 1);
}

void AuditLogWidget::log(const QString& action, const QString& module, int recordId, const QString& details)
{
    auto& db = DatabaseManager::instance();
    db.executeNonQuery(
        "INSERT INTO AuditLog (user_id, username, action, module, record_id, details) "
        "VALUES (0, 'admin', :action, :module, :rid, :details)",
        {{":action", action}, {":module", module}, {":rid", recordId > 0 ? recordId : QVariant()}, {":details", details}});
}

void AuditLogWidget::loadLogs()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute("SELECT * FROM AuditLog ORDER BY created_at DESC LIMIT 1000");

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM AuditLog");
    if (cc.next()) count = cc.value(0).toInt();
    m_table->setRowCount(qMin(count, 1000));
    m_table->setSortingEnabled(false);

    int row = 0;
    QMap<QString, int> actionCount;

    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        setItem(1, query.value("username").toString());

        QString action = query.value("action").toString();
        auto* actionItem = new QTableWidgetItem(action);
        actionItem->setTextAlignment(Qt::AlignCenter);
        if (action == "CREATE") actionItem->setForeground(QColor("#4ADE80"));
        else if (action == "UPDATE") actionItem->setForeground(QColor("#60A5FA"));
        else if (action == "DELETE") actionItem->setForeground(QColor("#E85454"));
        else if (action == "LOGIN") actionItem->setForeground(QColor("#FBBF24"));
        else actionItem->setForeground(QColor("#A78BFA"));
        m_table->setItem(row, 2, actionItem);
        actionCount[action]++;

        setItem(3, query.value("module").toString());
        setItem(4, query.value("record_id").toString());
        setItem(5, query.value("details").toString());
        setItem(6, query.value("created_at").toString());
        row++;
    }
    m_table->setSortingEnabled(true);
    int total = count;
    m_countLabel->setText(QString("Showing %1 of %2 logs").arg(row).arg(total));

    QString summary;
    for (auto it = actionCount.begin(); it != actionCount.end(); ++it)
        summary += it.key() + ": " + QString::number(it.value()) + " | ";
    if (!summary.isEmpty()) summary.chop(3);
    m_summaryLabel->setText(summary);
}

void AuditLogWidget::refresh() { loadLogs(); }

void AuditLogWidget::filterLogs(const QString& text)
{
    QString searchText = text.toLower();
    QString moduleFilter = m_moduleFilter->currentText();
    QString actionFilter = m_actionFilter->currentText();
    QDate fromDate = m_fromDate->date();

    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool textMatch = searchText.isEmpty();
        bool moduleMatch = (moduleFilter == "All");
        bool actionMatch = (actionFilter == "All");
        bool dateMatch = true;

        if (!textMatch) { for (int col = 1; col <= 5; ++col) { auto* item = m_table->item(row, col); if (item && item->text().toLower().contains(searchText)) { textMatch = true; break; } } }
        if (!moduleMatch) { auto* item = m_table->item(row, 3); if (item) moduleMatch = (item->text() == moduleFilter); }
        if (!actionMatch) { auto* item = m_table->item(row, 2); if (item) actionMatch = (item->text() == actionFilter); }

        auto* dateItem = m_table->item(row, 6);
        if (dateItem) {
            QDate logDate = QDateTime::fromString(dateItem->text(), "yyyy-MM-dd HH:mm:ss").date();
            if (logDate.isValid()) dateMatch = (logDate >= fromDate);
        }

        m_table->setRowHidden(row, !(textMatch && moduleMatch && actionMatch && dateMatch));
    }
}

void AuditLogWidget::clearOldLogs()
{
    auto result = QMessageBox::question(this, "Clear Old Logs",
        "Delete audit logs older than 90 days?\n\nThis cannot be undone.",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("DELETE FROM AuditLog WHERE created_at < datetime('now','localtime','-90 days')");
        loadLogs();
        QMessageBox::information(this, "Done", "Old audit logs cleared.");
    }
}

void AuditLogWidget::exportCSV()
{
    if (m_table->rowCount() == 0) { QMessageBox::information(this, "No Data", "No logs to export."); return; }
    QDir().mkpath(QCoreApplication::applicationDirPath() + "/reports");
    QString filePath = QFileDialog::getSaveFileName(this, "Export Audit Log", QCoreApplication::applicationDirPath() + "/reports/audit_log.csv", "CSV Files (*.csv);;All Files (*)");
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
    QMessageBox::information(this, "Export Successful", QString("Audit log exported to:\n\n%1").arg(filePath));
}
