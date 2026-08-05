#include "AnnouncementWidget.h"
#include "AnnouncementDialog.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>

AnnouncementWidget::AnnouncementWidget(QWidget* parent) : QWidget(parent) { buildUI(); loadAnnouncements(); }

void AnnouncementWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Announcements & Circulars");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);
    auto* subtitle = new QLabel("Send notices and circulars to all guards or specific sites");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(8);

    auto* addBtn = new QPushButton("+ New Announcement");
    addBtn->setObjectName("PrimaryButton");
    addBtn->setFixedSize(190, 36);
    addBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &AnnouncementWidget::addAnnouncement);
    headerRow->addWidget(addBtn);

    auto* editBtn = new QPushButton("Edit");
    editBtn->setObjectName("SecondaryButton");
    editBtn->setFixedSize(70, 36);
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, &AnnouncementWidget::editAnnouncement);
    headerRow->addWidget(editBtn);

    auto* delBtn = new QPushButton("Delete");
    delBtn->setObjectName("DangerButton");
    delBtn->setFixedSize(80, 36);
    delBtn->setCursor(Qt::PointingHandCursor);
    connect(delBtn, &QPushButton::clicked, this, &AnnouncementWidget::deleteAnnouncement);
    headerRow->addWidget(delBtn);

    headerRow->addStretch();
    m_countLabel = new QLabel;
    m_countLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    headerRow->addWidget(m_countLabel);
    mainLayout->addLayout(headerRow);

    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by title, message...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &AnnouncementWidget::filterAnnouncements);
    filterRow->addWidget(m_searchEdit, 1);

    m_priorityFilter = new QComboBox;
    m_priorityFilter->addItems({"All", "Normal", "Important", "Urgent"});
    m_priorityFilter->setFixedWidth(120);
    connect(m_priorityFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterAnnouncements(m_searchEdit->text()); });
    filterRow->addWidget(m_priorityFilter);

    m_statusFilter = new QComboBox;
    m_statusFilter->addItems({"All", "Active", "Draft", "Expired"});
    m_statusFilter->setFixedWidth(110);
    connect(m_statusFilter, &QComboBox::currentTextChanged, this, [this](const QString&) { filterAnnouncements(m_searchEdit->text()); });
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

    QStringList cols = {"ID", "Title", "Target", "Priority", "Published By", "Published", "Expires", "Status"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);
    m_table->setColumnWidth(1, 250);
    m_table->setColumnWidth(2, 130);
    m_table->setColumnWidth(3, 90);
    m_table->setColumnWidth(4, 130);
    m_table->setColumnWidth(5, 140);
    m_table->setColumnWidth(6, 140);
    m_table->horizontalHeader()->setStretchLastSection(true);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &AnnouncementWidget::editAnnouncement);
    mainLayout->addWidget(m_table, 1);
}

void AnnouncementWidget::loadAnnouncements()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute("SELECT * FROM Announcements ORDER BY published_at DESC");

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM Announcements");
    if (cc.next()) count = cc.value(0).toInt();
    m_table->setRowCount(count);
    m_table->setSortingEnabled(false);

    int row = 0, active = 0, urgent = 0;
    QDate today = QDate::currentDate();

    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        setItem(1, query.value("title").toString());

        QString target = query.value("target_type").toString();
        if (target != "All") target += " #" + query.value("target_id").toString();
        setItem(2, target);

        QString priority = query.value("priority").toString();
        auto* priItem = new QTableWidgetItem(priority);
        priItem->setTextAlignment(Qt::AlignCenter);
        if (priority == "Urgent") priItem->setForeground(QColor("#E85454"));
        else if (priority == "Important") priItem->setForeground(QColor("#FB923C"));
        else priItem->setForeground(QColor("#4ADE80"));
        m_table->setItem(row, 3, priItem);
        if (priority == "Urgent") urgent++;

        setItem(4, query.value("published_by").toString());
        setItem(5, query.value("published_at").toString());
        setItem(6, query.value("expires_at").toString());

        QString status = query.value("status").toString();
        // Auto-expire
        QDateTime expires = QDateTime::fromString(query.value("expires_at").toString(), "yyyy-MM-dd HH:mm");
        if (status == "Active" && expires.isValid() && expires < QDateTime::currentDateTime()) {
            status = "Expired";
            db.executeNonQuery("UPDATE Announcements SET status = 'Expired' WHERE id = :id",
                               {{":id", query.value("id").toInt()}});
        }

        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "Active") { statusItem->setForeground(QColor("#4ADE80")); active++; }
        else if (status == "Draft") statusItem->setForeground(QColor("#FBBF24"));
        else if (status == "Expired") statusItem->setForeground(QColor("#6B7585"));
        m_table->setItem(row, 7, statusItem);

        row++;
    }
    m_table->setSortingEnabled(true);
    m_countLabel->setText(QString("%1 announcements").arg(row));
    m_summaryLabel->setText(QString("Total: %1 | Active: %2 | Urgent: %3").arg(row).arg(active).arg(urgent));
}

void AnnouncementWidget::refresh() { loadAnnouncements(); }

void AnnouncementWidget::addAnnouncement()
{
    AnnouncementDialog dlg(this, -1);
    if (dlg.exec() == QDialog::Accepted) loadAnnouncements();
}

void AnnouncementWidget::editAnnouncement()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select an announcement to edit."); return; }
    int id = m_table->item(items.first()->row(), 0)->text().toInt();
    AnnouncementDialog dlg(this, id);
    if (dlg.exec() == QDialog::Accepted) loadAnnouncements();
}

void AnnouncementWidget::deleteAnnouncement()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select an announcement to delete."); return; }
    int row = items.first()->row();
    int id = m_table->item(row, 0)->text().toInt();
    QString title = m_table->item(row, 1)->text();
    auto result = QMessageBox::question(this, "Delete",
        QString("Delete announcement \"%1\"?").arg(title),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("DELETE FROM Announcements WHERE id = :id", {{":id", id}});
        loadAnnouncements();
    }
}

void AnnouncementWidget::filterAnnouncements(const QString& text)
{
    QString searchText = text.toLower();
    QString priFilter = m_priorityFilter->currentText();
    QString statusFilter = m_statusFilter->currentText();
    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool textMatch = searchText.isEmpty();
        bool priMatch = (priFilter == "All");
        bool statusMatch = (statusFilter == "All");
        if (!textMatch) { for (int col = 1; col <= 6; ++col) { auto* item = m_table->item(row, col); if (item && item->text().toLower().contains(searchText)) { textMatch = true; break; } } }
        if (!priMatch) { auto* item = m_table->item(row, 3); if (item) priMatch = (item->text() == priFilter); }
        if (!statusMatch) { auto* item = m_table->item(row, 7); if (item) statusMatch = (item->text() == statusFilter); }
        m_table->setRowHidden(row, !(textMatch && priMatch && statusMatch));
    }
}
