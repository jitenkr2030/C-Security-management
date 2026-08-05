#include "PhotoWidget.h"
#include "PhotoDialog.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QHeaderView>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QPixmap>

PhotoWidget::PhotoWidget(QWidget* parent) : QWidget(parent) { buildUI(); loadPhotos(); }

void PhotoWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Photo Gallery");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);
    auto* subtitle = new QLabel("Store guard photos, site photos, incident evidence and more");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(8);

    auto* addBtn = new QPushButton("+ Upload Photo");
    addBtn->setObjectName("PrimaryButton");
    addBtn->setFixedSize(150, 36);
    addBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &PhotoWidget::addPhoto);
    headerRow->addWidget(addBtn);

    auto* editBtn = new QPushButton("Edit");
    editBtn->setObjectName("SecondaryButton");
    editBtn->setFixedSize(70, 36);
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, &PhotoWidget::editPhoto);
    headerRow->addWidget(editBtn);

    auto* viewBtn = new QPushButton("View");
    viewBtn->setObjectName("SecondaryButton");
    viewBtn->setFixedSize(70, 36);
    viewBtn->setCursor(Qt::PointingHandCursor);
    viewBtn->setStyleSheet("QPushButton { background-color: #1A2A3A; color: #60A5FA; border: 1px solid #2A3A5A; border-radius: 6px; padding: 6px 16px; font-weight: 600; } QPushButton:hover { background-color: #2A3A5A; }");
    connect(viewBtn, &QPushButton::clicked, this, &PhotoWidget::viewPhoto);
    headerRow->addWidget(viewBtn);

    auto* delBtn = new QPushButton("Delete");
    delBtn->setObjectName("DangerButton");
    delBtn->setFixedSize(80, 36);
    delBtn->setCursor(Qt::PointingHandCursor);
    connect(delBtn, &QPushButton::clicked, this, &PhotoWidget::deletePhoto);
    headerRow->addWidget(delBtn);

    headerRow->addStretch();
    m_countLabel = new QLabel;
    m_countLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    headerRow->addWidget(m_countLabel);
    mainLayout->addLayout(headerRow);

    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by title, category...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &PhotoWidget::filterPhotos);
    filterRow->addWidget(m_searchEdit, 1);

    m_categoryFilter = new QComboBox;
    m_categoryFilter->addItems({"All", "Guard Photo", "Site Photo", "Incident Evidence",
                                "Training", "Uniform", "Equipment", "ID Card", "Other"});
    m_categoryFilter->setFixedWidth(160);
    connect(m_categoryFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterPhotos(m_searchEdit->text()); });
    filterRow->addWidget(m_categoryFilter);
    mainLayout->addLayout(filterRow);

    m_summaryLabel = new QLabel;
    m_summaryLabel->setStyleSheet("color: #D4B44C; font-size: 13px; font-weight: 600;");
    mainLayout->addWidget(m_summaryLabel);

    // Splitter: table on left, preview on right
    auto* splitter = new QSplitter(Qt::Horizontal);

    m_table = new QTableWidget;
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(true);

    QStringList cols = {"ID", "Category", "Title", "File", "Related", "Site", "Date", "By"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);
    m_table->setColumnWidth(1, 130);
    m_table->setColumnWidth(2, 180);
    m_table->setColumnWidth(3, 200);
    m_table->setColumnWidth(4, 100);
    m_table->setColumnWidth(5, 120);
    m_table->setColumnWidth(6, 100);
    m_table->horizontalHeader()->setStretchLastSection(true);
    connect(m_table, &QTableWidget::cellClicked, this, [this](int row, int) {
        if (row >= 0) {
            QString path = m_table->item(row, 3)->text();
            QFileInfo fi(path);
            if (fi.exists()) {
                QPixmap px(path);
                if (!px.isNull()) {
                    m_previewLabel->setPixmap(px.scaled(m_previewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    return;
                }
            }
            m_previewLabel->clear();
            m_previewLabel->setText("File not found");
        }
    });
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &PhotoWidget::viewPhoto);
    splitter->addWidget(m_table);

    // Preview panel
    auto* previewWidget = new QWidget;
    auto* previewLayout = new QVBoxLayout(previewWidget);
    previewLayout->setContentsMargins(8, 8, 8, 8);
    auto* previewTitle = new QLabel("Preview");
    previewTitle->setStyleSheet("color: #D4B44C; font-weight: 700; font-size: 14px;");
    previewLayout->addWidget(previewTitle);

    m_previewLabel = new QLabel("Click a photo to preview");
    m_previewLabel->setStyleSheet("background-color: #1E2530; border: 1px solid #2A3545; border-radius: 8px; color: #6B7585;");
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setMinimumSize(250, 200);
    previewLayout->addWidget(m_previewLabel, 1);

    splitter->addWidget(previewWidget);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    mainLayout->addWidget(splitter, 1);
}

void PhotoWidget::loadPhotos()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT p.*, s.site_name FROM Photos p "
        "LEFT JOIN Sites s ON p.site_id = s.id "
        "ORDER BY p.created_at DESC"
    );

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM Photos");
    if (cc.next()) count = cc.value(0).toInt();
    m_table->setRowCount(count);
    m_table->setSortingEnabled(false);

    int row = 0;
    QMap<QString, int> catCount;

    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());

        QString cat = query.value("category").toString();
        auto* catItem = new QTableWidgetItem(cat);
        if (cat == "Guard Photo") catItem->setForeground(QColor("#4ADE80"));
        else if (cat == "Incident Evidence") catItem->setForeground(QColor("#E85454"));
        else if (cat == "Site Photo") catItem->setForeground(QColor("#60A5FA"));
        else catItem->setForeground(QColor("#FBBF24"));
        m_table->setItem(row, 1, catItem);
        catCount[cat]++;

        setItem(2, query.value("title").toString());
        setItem(3, query.value("file_path").toString());

        QString related = query.value("related_type").toString();
        if (!related.isEmpty() && related != "None") related += " #" + query.value("related_id").toString();
        else related = "-";
        setItem(4, related);

        setItem(5, query.value("site_name").toString());
        setItem(6, query.value("taken_date").toString());
        setItem(7, query.value("uploaded_by").toString());
        row++;
    }
    m_table->setSortingEnabled(true);
    m_countLabel->setText(QString("%1 photos").arg(row));

    QString summary = "Total: " + QString::number(row);
    for (auto it = catCount.begin(); it != catCount.end(); ++it)
        summary += " | " + it.key() + ": " + QString::number(it.value());
    m_summaryLabel->setText(summary);
}

void PhotoWidget::refresh() { loadPhotos(); }

void PhotoWidget::addPhoto()
{
    PhotoDialog dlg(this, -1);
    if (dlg.exec() == QDialog::Accepted) loadPhotos();
}

void PhotoWidget::editPhoto()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a photo to edit."); return; }
    int id = m_table->item(items.first()->row(), 0)->text().toInt();
    PhotoDialog dlg(this, id);
    if (dlg.exec() == QDialog::Accepted) loadPhotos();
}

void PhotoWidget::viewPhoto()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a photo to view."); return; }
    QString path = m_table->item(items.first()->row(), 3)->text();
    QFileInfo fi(path);
    if (!fi.exists()) { QMessageBox::warning(this, "File Not Found", QString("File not found:\n%1").arg(path)); return; }
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void PhotoWidget::deletePhoto()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a photo to delete."); return; }
    int row = items.first()->row();
    int id = m_table->item(row, 0)->text().toInt();
    QString title = m_table->item(row, 2)->text();
    auto result = QMessageBox::question(this, "Delete",
        QString("Delete photo \"%1\"?\nThis will also delete the stored file.").arg(title),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (result == QMessageBox::Yes) {
        QString path = m_table->item(row, 3)->text();
        QFileInfo fi(path);
        if (fi.exists() && fi.absolutePath().contains("photos")) QFile::remove(path);
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("DELETE FROM Photos WHERE id = :id", {{":id", id}});
        loadPhotos();
    }
}

void PhotoWidget::filterPhotos(const QString& text)
{
    QString searchText = text.toLower();
    QString catFilter = m_categoryFilter->currentText();
    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool textMatch = searchText.isEmpty();
        bool catMatch = (catFilter == "All");
        if (!textMatch) { for (int col = 1; col <= 7; ++col) { auto* item = m_table->item(row, col); if (item && item->text().toLower().contains(searchText)) { textMatch = true; break; } } }
        if (!catMatch) { auto* item = m_table->item(row, 1); if (item) catMatch = (item->text() == catFilter); }
        m_table->setRowHidden(row, !(textMatch && catMatch));
    }
}
