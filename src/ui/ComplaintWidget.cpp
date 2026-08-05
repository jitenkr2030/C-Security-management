#include "ComplaintWidget.h"
#include "ComplaintDialog.h"
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

ComplaintWidget::ComplaintWidget(QWidget* parent) : QWidget(parent) { buildUI(); loadComplaints(); }

void ComplaintWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Complaints & Feedback");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);
    auto* subtitle = new QLabel("Track client complaints, guard feedback and resolution status");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(8);

    auto* addBtn = new QPushButton("+ New Complaint");
    addBtn->setObjectName("PrimaryButton");
    addBtn->setFixedSize(160, 36);
    addBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &ComplaintWidget::addComplaint);
    headerRow->addWidget(addBtn);

    auto* editBtn = new QPushButton("Edit");
    editBtn->setObjectName("SecondaryButton");
    editBtn->setFixedSize(70, 36);
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, &ComplaintWidget::editComplaint);
    headerRow->addWidget(editBtn);

    auto* resolveBtn = new QPushButton("Resolve");
    resolveBtn->setFixedSize(90, 36);
    resolveBtn->setCursor(Qt::PointingHandCursor);
    resolveBtn->setStyleSheet("QPushButton { background-color: #1A3A1A; color: #4ADE80; border: 1px solid #2A4A2A; border-radius: 6px; padding: 6px 16px; font-weight: 600; } QPushButton:hover { background-color: #2A4A2A; }");
    connect(resolveBtn, &QPushButton::clicked, this, &ComplaintWidget::resolveComplaint);
    headerRow->addWidget(resolveBtn);

    auto* delBtn = new QPushButton("Delete");
    delBtn->setObjectName("DangerButton");
    delBtn->setFixedSize(80, 36);
    delBtn->setCursor(Qt::PointingHandCursor);
    connect(delBtn, &QPushButton::clicked, this, &ComplaintWidget::deleteComplaint);
    headerRow->addWidget(delBtn);

    auto* exportBtn = new QPushButton("Export CSV");
    exportBtn->setObjectName("SecondaryButton");
    exportBtn->setFixedSize(110, 36);
    exportBtn->setCursor(Qt::PointingHandCursor);
    connect(exportBtn, &QPushButton::clicked, this, &ComplaintWidget::exportCSV);
    headerRow->addWidget(exportBtn);

    headerRow->addStretch();
    m_countLabel = new QLabel;
    m_countLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    headerRow->addWidget(m_countLabel);
    mainLayout->addLayout(headerRow);

    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by code, subject, complainant...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ComplaintWidget::filterComplaints);
    filterRow->addWidget(m_searchEdit, 1);

    m_typeFilter = new QComboBox;
    m_typeFilter->addItems({"All Types", "Client", "Guard", "Public", "Internal"});
    m_typeFilter->setFixedWidth(120);
    connect(m_typeFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterComplaints(m_searchEdit->text()); });
    filterRow->addWidget(m_typeFilter);

    m_statusFilter = new QComboBox;
    m_statusFilter->addItems({"All", "Open", "Under Review", "Resolved", "Closed", "Escalated"});
    m_statusFilter->setFixedWidth(140);
    connect(m_statusFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterComplaints(m_searchEdit->text()); });
    filterRow->addWidget(m_statusFilter);

    m_severityFilter = new QComboBox;
    m_severityFilter->addItems({"All", "Low", "Medium", "High", "Critical"});
    m_severityFilter->setFixedWidth(110);
    connect(m_severityFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterComplaints(m_searchEdit->text()); });
    filterRow->addWidget(m_severityFilter);
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

    QStringList cols = {"ID", "Code", "Type", "Category", "Subject", "Complainant",
                        "Client", "Site", "Severity", "Status", "Assigned To", "Created"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);
    m_table->setColumnWidth(1, 90);
    m_table->setColumnWidth(2, 80);
    m_table->setColumnWidth(3, 130);
    m_table->setColumnWidth(4, 200);
    m_table->setColumnWidth(5, 130);
    m_table->setColumnWidth(6, 130);
    m_table->setColumnWidth(7, 120);
    m_table->setColumnWidth(8, 80);
    m_table->setColumnWidth(9, 110);
    m_table->setColumnWidth(10, 120);
    m_table->horizontalHeader()->setStretchLastSection(true);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &ComplaintWidget::editComplaint);
    mainLayout->addWidget(m_table, 1);
}

void ComplaintWidget::loadComplaints()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT c.*, cl.client_name, s.site_name "
        "FROM Complaints c "
        "LEFT JOIN Clients cl ON c.client_id = cl.id "
        "LEFT JOIN Sites s ON c.site_id = s.id "
        "ORDER BY c.created_at DESC"
    );

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM Complaints");
    if (cc.next()) count = cc.value(0).toInt();
    m_table->setRowCount(count);
    m_table->setSortingEnabled(false);

    int row = 0, open = 0, escalated = 0, high = 0, resolved = 0;
    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        setItem(1, query.value("complaint_code").toString());
        setItem(2, query.value("complaint_type").toString());
        setItem(3, query.value("category").toString());
        setItem(4, query.value("subject").toString());
        setItem(5, query.value("complainant_name").toString());
        setItem(6, query.value("client_name").toString());
        setItem(7, query.value("site_name").toString());

        QString severity = query.value("severity").toString();
        auto* sevItem = new QTableWidgetItem(severity);
        sevItem->setTextAlignment(Qt::AlignCenter);
        if (severity == "Low") sevItem->setForeground(QColor("#60A5FA"));
        else if (severity == "Medium") sevItem->setForeground(QColor("#FBBF24"));
        else if (severity == "High") { sevItem->setForeground(QColor("#FB923C")); high++; }
        else if (severity == "Critical") { sevItem->setForeground(QColor("#E85454")); high++; }
        m_table->setItem(row, 8, sevItem);

        QString status = query.value("status").toString();
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "Open") { statusItem->setForeground(QColor("#FBBF24")); open++; }
        else if (status == "Under Review") statusItem->setForeground(QColor("#60A5FA"));
        else if (status == "Resolved") { statusItem->setForeground(QColor("#4ADE80")); resolved++; }
        else if (status == "Closed") statusItem->setForeground(QColor("#6B7585"));
        else if (status == "Escalated") { statusItem->setForeground(QColor("#E85454")); escalated++; }
        m_table->setItem(row, 9, statusItem);

        setItem(10, query.value("assigned_to").toString());
        setItem(11, query.value("created_at").toString());
        row++;
    }
    m_table->setSortingEnabled(true);
    m_countLabel->setText(QString("%1 complaints").arg(row));
    m_summaryLabel->setText(QString("Total: %1 | Open: %2 | Escalated: %3 | High/Critical: %4 | Resolved: %5")
        .arg(row).arg(open).arg(escalated).arg(high).arg(resolved));
}

void ComplaintWidget::refresh() { loadComplaints(); }

void ComplaintWidget::addComplaint()
{
    ComplaintDialog dlg(this, -1);
    if (dlg.exec() == QDialog::Accepted) loadComplaints();
}

void ComplaintWidget::editComplaint()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a complaint to edit."); return; }
    int id = m_table->item(items.first()->row(), 0)->text().toInt();
    ComplaintDialog dlg(this, id);
    if (dlg.exec() == QDialog::Accepted) loadComplaints();
}

void ComplaintWidget::resolveComplaint()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a complaint to resolve."); return; }
    int row = items.first()->row();
    int id = m_table->item(row, 0)->text().toInt();
    QString code = m_table->item(row, 1)->text();
    QString status = m_table->item(row, 9)->text();
    if (status == "Resolved" || status == "Closed") { QMessageBox::information(this, "Already Resolved", "This complaint is already resolved."); return; }

    bool ok;
    QString resolution = QInputDialog::getText(this, "Resolve Complaint",
        QString("Enter resolution for %1:").arg(code), QLineEdit::Normal, "", &ok);
    if (ok && !resolution.trimmed().isEmpty()) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("UPDATE Complaints SET status = 'Resolved', resolution = :res, resolved_date = datetime('now','localtime') WHERE id = :id",
            {{":res", resolution.trimmed()}, {":id", id}});
        loadComplaints();
    }
}

void ComplaintWidget::deleteComplaint()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a complaint to delete."); return; }
    int row = items.first()->row();
    int id = m_table->item(row, 0)->text().toInt();
    QString code = m_table->item(row, 1)->text();
    auto result = QMessageBox::question(this, "Delete",
        QString("Delete complaint \"%1\"?").arg(code),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("DELETE FROM Complaints WHERE id = :id", {{":id", id}});
        loadComplaints();
    }
}

void ComplaintWidget::filterComplaints(const QString& text)
{
    QString searchText = text.toLower();
    QString typeFilter = m_typeFilter->currentText();
    QString statusFilter = m_statusFilter->currentText();
    QString sevFilter = m_severityFilter->currentText();
    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool textMatch = searchText.isEmpty();
        bool typeMatch = (typeFilter == "All Types");
        bool statusMatch = (statusFilter == "All");
        bool sevMatch = (sevFilter == "All");
        if (!textMatch) { for (int col = 1; col <= 7; ++col) { auto* item = m_table->item(row, col); if (item && item->text().toLower().contains(searchText)) { textMatch = true; break; } } }
        if (!typeMatch) { auto* item = m_table->item(row, 2); if (item) typeMatch = (item->text() == typeFilter); }
        if (!statusMatch) { auto* item = m_table->item(row, 9); if (item) statusMatch = (item->text() == statusFilter); }
        if (!sevMatch) { auto* item = m_table->item(row, 8); if (item) sevMatch = (item->text() == sevFilter); }
        m_table->setRowHidden(row, !(textMatch && typeMatch && statusMatch && sevMatch));
    }
}

void ComplaintWidget::exportCSV()
{
    if (m_table->rowCount() == 0) { QMessageBox::information(this, "No Data", "No complaints to export."); return; }
    QDir().mkpath(QCoreApplication::applicationDirPath() + "/reports");
    QString filePath = QFileDialog::getSaveFileName(this, "Export Complaints", QCoreApplication::applicationDirPath() + "/reports/complaints.csv", "CSV Files (*.csv);;All Files (*)");
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
    QMessageBox::information(this, "Export Successful", QString("Complaints exported to:\n\n%1").arg(filePath));
}
