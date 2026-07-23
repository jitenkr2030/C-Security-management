#include "DocumentWidget.h"
#include "DocumentDialog.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>

DocumentWidget::DocumentWidget(QWidget* parent) : QWidget(parent) { buildUI(); loadDocuments(); }

void DocumentWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Document Management");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);
    auto* subtitle = new QLabel("Store and manage guard and client documents with file storage");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(8);

    auto* addBtn = new QPushButton("+ Upload Document");
    addBtn->setObjectName("PrimaryButton");
    addBtn->setFixedSize(170, 36);
    addBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &DocumentWidget::addDocument);
    headerRow->addWidget(addBtn);

    auto* editBtn = new QPushButton("Edit");
    editBtn->setObjectName("SecondaryButton");
    editBtn->setFixedSize(70, 36);
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, &DocumentWidget::editDocument);
    headerRow->addWidget(editBtn);

    auto* openBtn = new QPushButton("Open File");
    openBtn->setObjectName("SecondaryButton");
    openBtn->setFixedSize(100, 36);
    openBtn->setCursor(Qt::PointingHandCursor);
    openBtn->setStyleSheet("QPushButton { background-color: #1A2A3A; color: #60A5FA; border: 1px solid #2A3A5A; border-radius: 6px; padding: 6px 16px; font-weight: 600; } QPushButton:hover { background-color: #2A3A5A; }");
    connect(openBtn, &QPushButton::clicked, this, &DocumentWidget::openDocument);
    headerRow->addWidget(openBtn);

    auto* delBtn = new QPushButton("Delete");
    delBtn->setObjectName("DangerButton");
    delBtn->setFixedSize(80, 36);
    delBtn->setCursor(Qt::PointingHandCursor);
    connect(delBtn, &QPushButton::clicked, this, &DocumentWidget::deleteDocument);
    headerRow->addWidget(delBtn);

    headerRow->addStretch();
    m_countLabel = new QLabel;
    m_countLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    headerRow->addWidget(m_countLabel);
    mainLayout->addLayout(headerRow);

    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by file name, type, description...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &DocumentWidget::filterDocuments);
    filterRow->addWidget(m_searchEdit, 1);

    m_typeFilter = new QComboBox;
    m_typeFilter->addItems({"All Types", "Aadhaar Card", "PAN Card", "Resume", "Police Verification", "Medical Certificate", "Training Certificate", "Contract", "Insurance", "ID Card", "Photo", "Other"});
    m_typeFilter->setFixedWidth(170);
    connect(m_typeFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterDocuments(m_searchEdit->text()); });
    filterRow->addWidget(m_typeFilter);

    m_entityFilter = new QComboBox;
    m_entityFilter->addItems({"All", "Guard Docs", "Client Docs", "General"});
    m_entityFilter->setFixedWidth(120);
    connect(m_entityFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterDocuments(m_searchEdit->text()); });
    filterRow->addWidget(m_entityFilter);
    mainLayout->addLayout(filterRow);

    m_table = new QTableWidget;
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(true);

    QStringList cols = {"ID", "Guard ID", "Client ID", "Guard", "Client", "Type", "File Name", "File Path", "Size", "Description", "Uploaded"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);
    m_table->setColumnHidden(1, true);
    m_table->setColumnHidden(2, true);
    m_table->setColumnHidden(7, true); // Hide raw path
    m_table->setColumnWidth(3, 150);
    m_table->setColumnWidth(4, 150);
    m_table->setColumnWidth(5, 130);
    m_table->setColumnWidth(6, 200);
    m_table->setColumnWidth(8, 80);
    m_table->setColumnWidth(9, 200);
    m_table->horizontalHeader()->setStretchLastSection(true);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &DocumentWidget::openDocument);
    mainLayout->addWidget(m_table, 1);
}

void DocumentWidget::loadDocuments()
{
    auto& db = DatabaseManager::instance();

    // Check if Documents table exists and has data
    auto cc = db.execute("SELECT COUNT(*) FROM Documents");
    int count = 0;
    if (cc.next()) count = cc.value(0).toInt();

    m_table->setRowCount(count);
    m_table->setSortingEnabled(false);

    if (count == 0) {
        m_table->setSortingEnabled(true);
        m_countLabel->setText("0 documents");
        return;
    }

    auto query = db.execute(
        "SELECT d.*, "
        "COALESCE(g.full_name,'') AS guard_name, "
        "COALESCE(c.client_name,'') AS client_name "
        "FROM Documents d "
        "LEFT JOIN Guards g ON d.guard_id = g.id "
        "LEFT JOIN Clients c ON d.client_id = c.id "
        "ORDER BY d.created_at DESC"
    );

    int row = 0;
    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };
        setItem(0, query.value("id").toString());
        setItem(1, query.value("guard_id").toString());
        setItem(2, query.value("client_id").toString());
        setItem(3, query.value("guard_name").toString());
        setItem(4, query.value("client_name").toString());
        setItem(5, query.value("document_type").toString());
        setItem(6, query.value("file_name").toString());
        setItem(7, query.value("file_path").toString());

        QString filePath = query.value("file_path").toString();
        QFileInfo fi(filePath);
        QString sizeStr = "-";
        if (fi.exists()) {
            double sizeKB = fi.size() / 1024.0;
            if (sizeKB >= 1024) sizeStr = QString::number(sizeKB / 1024.0, 'f', 1) + " MB";
            else sizeStr = QString::number(sizeKB, 'f', 1) + " KB";
        }
        auto* sizeItem = new QTableWidgetItem(sizeStr);
        sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (!fi.exists() && !filePath.isEmpty()) sizeItem->setForeground(QColor("#E85454"));
        m_table->setItem(row, 8, sizeItem);

        setItem(9, query.value("description").toString());
        setItem(10, query.value("created_at").toString());
        row++;
    }
    m_table->setSortingEnabled(true);
    m_countLabel->setText(QString("%1 documents").arg(row));
}

void DocumentWidget::refresh() { loadDocuments(); }

void DocumentWidget::addDocument()
{
    DocumentDialog dlg(this, -1);
    if (dlg.exec() == QDialog::Accepted) loadDocuments();
}

void DocumentWidget::editDocument()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a document to edit."); return; }
    int id = m_table->item(items.first()->row(), 0)->text().toInt();
    DocumentDialog dlg(this, id);
    if (dlg.exec() == QDialog::Accepted) loadDocuments();
}

void DocumentWidget::openDocument()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a document to open."); return; }
    int row = items.first()->row();
    QString filePath = m_table->item(row, 7)->text();
    QString fileName = m_table->item(row, 6)->text();

    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "No File", "No file path stored for this document.");
        return;
    }

    QFileInfo fi(filePath);
    if (!fi.exists()) {
        QMessageBox::warning(this, "File Not Found",
            QString("File not found:\n\n%1\n\nThe file may have been moved or deleted.").arg(filePath));
        return;
    }

    bool opened = QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    if (!opened) {
        QMessageBox::warning(this, "Cannot Open",
            QString("Could not open file:\n\n%1\n\nNo application found for this file type.").arg(filePath));
    }
}

void DocumentWidget::deleteDocument()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a document to delete."); return; }
    int row = items.first()->row();
    int id = m_table->item(row, 0)->text().toInt();
    QString name = m_table->item(row, 6)->text();
    QString filePath = m_table->item(row, 7)->text();

    auto result = QMessageBox::question(this, "Delete",
        QString("Delete document \"%1\"?\n\nThis will also delete the stored file.").arg(name),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (result == QMessageBox::Yes) {
        // Delete the actual file if it exists
        if (!filePath.isEmpty()) {
            QFileInfo fi(filePath);
            if (fi.exists() && fi.absolutePath().contains("documents")) {
                QFile::remove(filePath);
            }
        }
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("DELETE FROM Documents WHERE id = :id", {{":id", id}});
        loadDocuments();
    }
}

void DocumentWidget::filterDocuments(const QString& text)
{
    QString searchText = text.toLower();
    QString typeFilter = m_typeFilter->currentText();
    QString entityFilter = m_entityFilter->currentText();
    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool textMatch = searchText.isEmpty();
        bool typeMatch = (typeFilter == "All Types");
        bool entityMatch = (entityFilter == "All");
        if (!textMatch) { for (int col = 3; col <= 9; ++col) { auto* item = m_table->item(row, col); if (item && item->text().toLower().contains(searchText)) { textMatch = true; break; } } }
        if (!typeMatch) { auto* item = m_table->item(row, 5); if (item) typeMatch = (item->text() == typeFilter); }
        if (!entityMatch) {
            if (entityFilter == "Guard Docs") { auto* item = m_table->item(row, 1); entityMatch = (item && !item->text().isEmpty() && item->text() != "0"); }
            else if (entityFilter == "Client Docs") { auto* item = m_table->item(row, 2); entityMatch = (item && !item->text().isEmpty() && item->text() != "0"); }
            else if (entityFilter == "General") { auto* g = m_table->item(row, 1); auto* c = m_table->item(row, 2); bool hasGuard = g && !g->text().isEmpty() && g->text() != "0"; bool hasClient = c && !c->text().isEmpty() && c->text() != "0"; entityMatch = !hasGuard && !hasClient; }
        }
        m_table->setRowHidden(row, !(textMatch && typeMatch && entityMatch));
    }
}
