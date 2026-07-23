#include "IncidentWidget.h"
#include "IncidentDialog.h"
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
#include <QDesktopServices>
#include <QUrl>

IncidentWidget::IncidentWidget(QWidget* parent) : QWidget(parent) { buildUI(); loadIncidents(); }

void IncidentWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Incident Register");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);
    auto* subtitle = new QLabel("Report, track and resolve security incidents with photo evidence");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(8);

    auto* addBtn = new QPushButton("+ Report Incident");
    addBtn->setObjectName("PrimaryButton");
    addBtn->setFixedSize(160, 36);
    addBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &IncidentWidget::addIncident);
    headerRow->addWidget(addBtn);

    auto* editBtn = new QPushButton("Edit");
    editBtn->setObjectName("SecondaryButton");
    editBtn->setFixedSize(70, 36);
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, &IncidentWidget::editIncident);
    headerRow->addWidget(editBtn);

    auto* resolveBtn = new QPushButton("Resolve");
    resolveBtn->setFixedSize(90, 36);
    resolveBtn->setCursor(Qt::PointingHandCursor);
    resolveBtn->setStyleSheet("QPushButton { background-color: #1A3A1A; color: #4ADE80; border: 1px solid #2A4A2A; border-radius: 6px; padding: 6px 16px; font-weight: 600; } QPushButton:hover { background-color: #2A4A2A; }");
    connect(resolveBtn, &QPushButton::clicked, this, &IncidentWidget::resolveIncident);
    headerRow->addWidget(resolveBtn);

    auto* delBtn = new QPushButton("Delete");
    delBtn->setObjectName("DangerButton");
    delBtn->setFixedSize(80, 36);
    delBtn->setCursor(Qt::PointingHandCursor);
    connect(delBtn, &QPushButton::clicked, this, &IncidentWidget::deleteIncident);
    headerRow->addWidget(delBtn);

    auto* exportBtn = new QPushButton("Export CSV");
    exportBtn->setObjectName("SecondaryButton");
    exportBtn->setFixedSize(110, 36);
    exportBtn->setCursor(Qt::PointingHandCursor);
    connect(exportBtn, &QPushButton::clicked, this, &IncidentWidget::exportCSV);
    headerRow->addWidget(exportBtn);

    headerRow->addStretch();
    m_countLabel = new QLabel;
    m_countLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    headerRow->addWidget(m_countLabel);
    mainLayout->addLayout(headerRow);

    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by code, type, description...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &IncidentWidget::filterIncidents);
    filterRow->addWidget(m_searchEdit, 1);

    m_statusFilter = new QComboBox;
    m_statusFilter->addItems({"All", "Open", "Under Investigation", "Resolved", "Closed"});
    m_statusFilter->setFixedWidth(170);
    connect(m_statusFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterIncidents(m_searchEdit->text()); });
    filterRow->addWidget(m_statusFilter);

    m_severityFilter = new QComboBox;
    m_severityFilter->addItems({"All", "Low", "Medium", "High", "Critical"});
    m_severityFilter->setFixedWidth(110);
    connect(m_severityFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterIncidents(m_searchEdit->text()); });
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

    QStringList cols = {"ID", "Code", "Type", "Severity", "Site", "Date/Time",
                        "Description", "Reported By", "Status", "Photo", "Document", "Resolution"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);
    m_table->setColumnWidth(1, 90);
    m_table->setColumnWidth(2, 110);
    m_table->setColumnWidth(3, 80);
    m_table->setColumnWidth(4, 120);
    m_table->setColumnWidth(5, 140);
    m_table->setColumnWidth(6, 220);
    m_table->setColumnWidth(7, 110);
    m_table->setColumnWidth(8, 130);
    m_table->setColumnWidth(9, 60);
    m_table->setColumnWidth(10, 70);
    m_table->horizontalHeader()->setStretchLastSection(true);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &IncidentWidget::editIncident);
    mainLayout->addWidget(m_table, 1);
}

void IncidentWidget::loadIncidents()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute("SELECT i.*, s.site_name FROM Incidents i LEFT JOIN Sites s ON i.site_id = s.id ORDER BY i.date_time DESC");
    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM Incidents");
    if (cc.next()) count = cc.value(0).toInt();
    m_table->setRowCount(count);
    m_table->setSortingEnabled(false);

    int row = 0, open = 0, investigating = 0, resolved = 0, critical = 0;
    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };
        setItem(0, query.value("id").toString());
        setItem(1, query.value("incident_code").toString());
        setItem(2, query.value("incident_type").toString());

        QString severity = query.value("severity").toString();
        auto* sevItem = new QTableWidgetItem(severity);
        sevItem->setTextAlignment(Qt::AlignCenter);
        if (severity == "Low") sevItem->setForeground(QColor("#60A5FA"));
        else if (severity == "Medium") sevItem->setForeground(QColor("#FBBF24"));
        else if (severity == "High") sevItem->setForeground(QColor("#FB923C"));
        else if (severity == "Critical") { sevItem->setForeground(QColor("#E85454")); critical++; }
        m_table->setItem(row, 3, sevItem);

        setItem(4, query.value("site_name").toString());
        setItem(5, query.value("date_time").toString());
        setItem(6, query.value("description").toString());
        setItem(7, query.value("reported_by").toString());

        QString status = query.value("status").toString();
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "Open") { statusItem->setForeground(QColor("#FBBF24")); open++; }
        else if (status == "Under Investigation") { statusItem->setForeground(QColor("#60A5FA")); investigating++; }
        else if (status == "Resolved") { statusItem->setForeground(QColor("#4ADE80")); resolved++; }
        else if (status == "Closed") statusItem->setForeground(QColor("#6B7585"));
        m_table->setItem(row, 8, statusItem);

        // Photo indicator
        QString photoPath = query.value("photo_path").toString();
        auto* photoItem = new QTableWidgetItem(photoPath.isEmpty() ? "-" : "Yes");
        photoItem->setTextAlignment(Qt::AlignCenter);
        if (!photoPath.isEmpty()) {
            photoItem->setForeground(QColor("#60A5FA"));
            photoItem->setToolTip(photoPath);
        }
        m_table->setItem(row, 9, photoItem);

        // Document indicator
        QString docPath = query.value("document_path").toString();
        auto* docItem = new QTableWidgetItem(docPath.isEmpty() ? "-" : "Yes");
        docItem->setTextAlignment(Qt::AlignCenter);
        if (!docPath.isEmpty()) {
            docItem->setForeground(QColor("#A78BFA"));
            docItem->setToolTip(docPath);
        }
        m_table->setItem(row, 10, docItem);

        setItem(11, query.value("resolution").toString());
        row++;
    }
    m_table->setSortingEnabled(true);
    m_countLabel->setText(QString("%1 incidents").arg(row));
    m_summaryLabel->setText(QString("Total: %1 | Open: %2 | Investigating: %3 | Resolved: %4 | Critical: %5").arg(row).arg(open).arg(investigating).arg(resolved).arg(critical));
}

void IncidentWidget::refresh() { loadIncidents(); }

void IncidentWidget::addIncident()
{
    IncidentDialog dlg(this, -1);
    if (dlg.exec() == QDialog::Accepted) loadIncidents();
}

void IncidentWidget::editIncident()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select an incident to edit."); return; }
    int id = m_table->item(items.first()->row(), 0)->text().toInt();
    IncidentDialog dlg(this, id);
    if (dlg.exec() == QDialog::Accepted) loadIncidents();
}

void IncidentWidget::resolveIncident()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select an incident to resolve."); return; }
    int row = items.first()->row();
    int id = m_table->item(row, 0)->text().toInt();
    QString code = m_table->item(row, 1)->text();
    QString status = m_table->item(row, 8)->text();
    if (status == "Resolved" || status == "Closed") { QMessageBox::information(this, "Already Resolved", "This incident is already resolved."); return; }

    bool ok;
    QString resolution = QInputDialog::getText(this, "Resolve Incident", QString("Enter resolution for %1:").arg(code), QLineEdit::Normal, "", &ok);
    if (ok && !resolution.trimmed().isEmpty()) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("UPDATE Incidents SET status = 'Resolved', resolution = :res WHERE id = :id", {{":res", resolution.trimmed()}, {":id", id}});
        loadIncidents();
    }
}

void IncidentWidget::deleteIncident()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select an incident to delete."); return; }
    int row = items.first()->row();
    int id = m_table->item(row, 0)->text().toInt();
    QString code = m_table->item(row, 1)->text();
    auto result = QMessageBox::question(this, "Delete", QString("Delete incident \"%1\"?").arg(code), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("DELETE FROM Incidents WHERE id = :id", {{":id", id}});
        loadIncidents();
    }
}

void IncidentWidget::filterIncidents(const QString& text)
{
    QString searchText = text.toLower();
    QString statusFilter = m_statusFilter->currentText();
    QString sevFilter = m_severityFilter->currentText();
    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool textMatch = searchText.isEmpty();
        bool statusMatch = (statusFilter == "All");
        bool sevMatch = (sevFilter == "All");
        if (!textMatch) { for (int col = 1; col <= 7; ++col) { auto* item = m_table->item(row, col); if (item && item->text().toLower().contains(searchText)) { textMatch = true; break; } } }
        if (!statusMatch) { auto* item = m_table->item(row, 8); if (item) statusMatch = (item->text() == statusFilter); }
        if (!sevMatch) { auto* item = m_table->item(row, 3); if (item) sevMatch = (item->text() == sevFilter); }
        m_table->setRowHidden(row, !(textMatch && statusMatch && sevMatch));
    }
}

void IncidentWidget::exportCSV()
{
    if (m_table->rowCount() == 0) { QMessageBox::information(this, "No Data", "No incident data to export."); return; }
    QDir().mkpath(QCoreApplication::applicationDirPath() + "/reports");
    QString filePath = QFileDialog::getSaveFileName(this, "Export Incident Report", QCoreApplication::applicationDirPath() + "/reports/incidents.csv", "CSV Files (*.csv);;All Files (*)");
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
    QMessageBox::information(this, "Export Successful", QString("Incident report exported to:\n\n%1").arg(filePath));
}
