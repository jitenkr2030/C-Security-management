#include "SiteWidget.h"
#include "SiteDialog.h"
#include "database/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFrame>

SiteWidget::SiteWidget(QWidget* parent)
    : QWidget(parent)
{
    buildUI();
    loadSites();
}

void SiteWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    // ---- Header ----
    auto* headerRow = new QHBoxLayout;

    auto* leftHeader = new QVBoxLayout;
    auto* titleLabel = new QLabel("Site Management");
    titleLabel->setObjectName("PageTitle");
    leftHeader->addWidget(titleLabel);

    m_countLabel = new QLabel("0 sites");
    m_countLabel->setObjectName("PageSubtitle");
    leftHeader->addWidget(m_countLabel);

    headerRow->addLayout(leftHeader);
    headerRow->addStretch();

    m_addBtn = new QPushButton("+ Add Site");
    m_addBtn->setObjectName("PrimaryButton");
    m_addBtn->setFixedSize(130, 40);
    m_addBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addBtn, &QPushButton::clicked, this, &SiteWidget::addSite);
    headerRow->addWidget(m_addBtn);

    mainLayout->addLayout(headerRow);

    // ---- Search / Filter ----
    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by site name, code, address, city...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &SiteWidget::filterSites);
    filterRow->addWidget(m_searchEdit, 1);

    m_clientFilter = new QComboBox;
    m_clientFilter->addItem("All Clients", 0);
    m_clientFilter->setFixedWidth(180);
    connect(m_clientFilter, &QComboBox::currentTextChanged, this,
            [this](const QString&) { filterSites(m_searchEdit->text()); });
    filterRow->addWidget(m_clientFilter);

    m_statusFilter = new QComboBox;
    m_statusFilter->addItems({
        "All Status", "Active", "Inactive",
        "Under Construction", "Contract Ended"
    });
    m_statusFilter->setFixedWidth(170);
    connect(m_statusFilter, &QComboBox::currentTextChanged, this,
            [this](const QString&) { filterSites(m_searchEdit->text()); });
    filterRow->addWidget(m_statusFilter);

    mainLayout->addLayout(filterRow);

    // ---- Action buttons ----
    auto* actionRow = new QHBoxLayout;
    actionRow->setSpacing(8);

    m_editBtn = new QPushButton("Edit");
    m_editBtn->setObjectName("SecondaryButton");
    m_editBtn->setFixedSize(80, 36);
    m_editBtn->setCursor(Qt::PointingHandCursor);
    connect(m_editBtn, &QPushButton::clicked, this, &SiteWidget::editSite);

    m_viewBtn = new QPushButton("View Details");
    m_viewBtn->setObjectName("SecondaryButton");
    m_viewBtn->setFixedSize(120, 36);
    m_viewBtn->setCursor(Qt::PointingHandCursor);
    connect(m_viewBtn, &QPushButton::clicked, this, &SiteWidget::viewSite);

    m_deleteBtn = new QPushButton("Delete");
    m_deleteBtn->setObjectName("DangerButton");
    m_deleteBtn->setFixedSize(90, 36);
    m_deleteBtn->setCursor(Qt::PointingHandCursor);
    connect(m_deleteBtn, &QPushButton::clicked, this, &SiteWidget::deleteSite);

    actionRow->addWidget(m_editBtn);
    actionRow->addWidget(m_viewBtn);
    actionRow->addWidget(m_deleteBtn);
    actionRow->addStretch();

    mainLayout->addLayout(actionRow);

    // ---- Table ----
    m_table = new QTableWidget;
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(true);

    QStringList columns = {
        "ID", "Code", "Site Name", "City", "Client",
        "Supervisor", "Guards Req.", "Assigned", "Morning", "Afternoon", "Night", "Status"
    };
    m_table->setColumnCount(columns.size());
    m_table->setHorizontalHeaderLabels(columns);
    m_table->setColumnHidden(0, true);

    auto* header = m_table->horizontalHeader();
    header->setStretchLastSection(true);
    header->setSectionResizeMode(2, QHeaderView::Stretch);

    m_table->setColumnWidth(1, 85);
    m_table->setColumnWidth(2, 180);
    m_table->setColumnWidth(3, 100);
    m_table->setColumnWidth(4, 150);
    m_table->setColumnWidth(5, 140);
    m_table->setColumnWidth(6, 95);
    m_table->setColumnWidth(7, 85);
    m_table->setColumnWidth(8, 105);
    m_table->setColumnWidth(9, 105);
    m_table->setColumnWidth(10, 105);
    m_table->setColumnWidth(11, 100);

    connect(m_table, &QTableWidget::cellDoubleClicked, this, &SiteWidget::editSite);

    mainLayout->addWidget(m_table, 1);
}

void SiteWidget::loadSites()
{
    auto& db = DatabaseManager::instance();

    // Load client filter options
    m_clientFilter->blockSignals(true);
    int currentClient = m_clientFilter->currentData().toInt();
    m_clientFilter->clear();
    m_clientFilter->addItem("All Clients", 0);
    QSqlQuery clients = db.execute("SELECT id, company_name FROM Clients ORDER BY company_name");
    while (clients.next()) {
        m_clientFilter->addItem(
            clients.value("company_name").toString(),
            clients.value("id").toInt()
        );
    }
    // Restore selection
    for (int i = 0; i < m_clientFilter->count(); ++i) {
        if (m_clientFilter->itemData(i).toInt() == currentClient) {
            m_clientFilter->setCurrentIndex(i);
            break;
        }
    }
    m_clientFilter->blockSignals(false);

    // Load sites with client name, supervisor name, and assigned guard count
    QSqlQuery query = db.execute(
        "SELECT s.*, "
        "c.company_name AS client_name, "
        "g.full_name AS supervisor_name, "
        "(SELECT COUNT(*) FROM Guards gu WHERE gu.site_id = s.id AND gu.status = 'Active') AS assigned_guards "
        "FROM Sites s "
        "LEFT JOIN Clients c ON s.client_id = c.id "
        "LEFT JOIN Guards g ON s.supervisor_id = g.id "
        "ORDER BY s.site_name"
    );

    m_table->setSortingEnabled(false);

    // Count rows
    int count = 0;
    QSqlQuery countQuery = db.execute("SELECT COUNT(*) FROM Sites");
    if (countQuery.next()) count = countQuery.value(0).toInt();
    m_table->setRowCount(count);

    int row = 0;
    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        setItem(1, query.value("site_code").toString());
        setItem(2, query.value("site_name").toString());
        setItem(3, query.value("city").toString());
        setItem(4, query.value("client_name").toString());
        setItem(5, query.value("supervisor_name").toString());
        setItem(6, query.value("guards_required").toString());

        int assigned = query.value("assigned_guards").toInt();
        int required = query.value("guards_required").toInt();
        auto* assignedItem = new QTableWidgetItem(QString::number(assigned));
        assignedItem->setTextAlignment(Qt::AlignCenter);

        if (assigned >= required) {
            assignedItem->setForeground(QColor("#4ADE80"));
        } else if (assigned > 0) {
            assignedItem->setForeground(QColor("#FBBF24"));
        } else {
            assignedItem->setForeground(QColor("#E85454"));
        }
        m_table->setItem(row, 7, assignedItem);

        setItem(8, query.value("shift_morning").toString());
        setItem(9, query.value("shift_afternoon").toString());
        setItem(10, query.value("shift_night").toString());
        setItem(11, query.value("status").toString());

        // Color-code status
        auto* statusItem = m_table->item(row, 11);
        QString status = query.value("status").toString();
        if (status == "Active") {
            statusItem->setForeground(QColor("#4ADE80"));
        } else if (status == "Inactive" || status == "Contract Ended") {
            statusItem->setForeground(QColor("#E85454"));
        } else if (status == "Under Construction") {
            statusItem->setForeground(QColor("#FBBF24"));
        }

        row++;
    }

    m_table->setSortingEnabled(true);
    m_countLabel->setText(QString("%1 sites").arg(row));
}

void SiteWidget::refresh()
{
    loadSites();
}

int SiteWidget::selectedSiteId() const
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) return -1;
    int row = items.first()->row();
    return m_table->item(row, 0)->text().toInt();
}

void SiteWidget::addSite()
{
    SiteDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        loadSites();
    }
}

void SiteWidget::editSite()
{
    int id = selectedSiteId();
    if (id < 0) {
        QMessageBox::information(this, "No Selection", "Please select a site to edit.");
        return;
    }
    SiteDialog dlg(this, id);
    if (dlg.exec() == QDialog::Accepted) {
        loadSites();
    }
}

void SiteWidget::deleteSite()
{
    int id = selectedSiteId();
    if (id < 0) {
        QMessageBox::information(this, "No Selection", "Please select a site to delete.");
        return;
    }

    int row = m_table->currentRow();
    QString name = m_table->item(row, 2)->text();

    auto result = QMessageBox::question(
        this, "Confirm Delete",
        QString("Are you sure you want to delete site \"%1\"?\n\n"
                "Guard assignments to this site will be cleared.").arg(name),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery(
            "UPDATE Guards SET site_id = NULL WHERE site_id = :id", {{":id", id}}
        );
        if (db.executeNonQuery("DELETE FROM Sites WHERE id = :id", {{":id", id}})) {
            loadSites();
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete site.");
        }
    }
}

void SiteWidget::viewSite()
{
    int id = selectedSiteId();
    if (id < 0) {
        QMessageBox::information(this, "No Selection", "Please select a site to view.");
        return;
    }

    auto& db = DatabaseManager::instance();

    // Get site details
    QSqlQuery siteQuery = db.execute(
        "SELECT s.*, c.company_name AS client_name, g.full_name AS supervisor_name "
        "FROM Sites s "
        "LEFT JOIN Clients c ON s.client_id = c.id "
        "LEFT JOIN Guards g ON s.supervisor_id = g.id "
        "WHERE s.id = :id",
        {{":id", id}}
    );
    if (!siteQuery.next()) return;

    // Count assigned guards
    QSqlQuery guardCount = db.execute(
        "SELECT COUNT(*) FROM Guards WHERE site_id = :id AND status = 'Active'",
        {{":id", id}}
    );
    int assigned = guardCount.next() ? guardCount.value(0).toInt() : 0;

    // Get list of assigned guards
    QSqlQuery guardList = db.execute(
        "SELECT guard_code, full_name, mobile_primary FROM Guards "
        "WHERE site_id = :id AND status = 'Active' ORDER BY full_name",
        {{":id", id}}
    );

    QString guardDetails;
    int gNum = 1;
    while (guardList.next()) {
        guardDetails += QString("  %1. %2 (%3) - %4\n")
            .arg(gNum++)
            .arg(guardList.value("full_name").toString())
            .arg(guardList.value("guard_code").toString())
            .arg(guardList.value("mobile_primary").toString());
    }
    if (guardDetails.isEmpty()) {
        guardDetails = "  No guards assigned\n";
    }

    // Today's attendance for this site
    QSqlQuery attQuery = db.execute(
        "SELECT "
        "(SELECT COUNT(*) FROM Attendance WHERE site_id = :id1 "
        "AND date = date('now','localtime') AND status = 'Present') AS present, "
        "(SELECT COUNT(*) FROM Attendance WHERE site_id = :id2 "
        "AND date = date('now','localtime') AND status = 'Absent') AS absent",
        {{":id1", id}, {":id2", id}}
    );
    int presentToday = 0, absentToday = 0;
    if (attQuery.next()) {
        presentToday = attQuery.value("present").toInt();
        absentToday = attQuery.value("absent").toInt();
    }

    int required = siteQuery.value("guards_required").toInt();
    QString fillStatus = assigned >= required ? "FULLY STAFFED" : "UNDERSTAFFED";

    QString info = QString(
        "Site Details\n"
        "================================================\n"
        "Code:             %1\n"
        "Site Name:        %2\n"
        "Address:          %3\n"
        "City:             %4\n"
        "Client:           %5\n"
        "Status:           %6\n"
        "================================================\n"
        "Shift Timings:\n"
        "  Morning:        %7\n"
        "  Afternoon:      %8\n"
        "  Night:          %9\n"
        "================================================\n"
        "Supervisor:       %10\n"
        "Guards Required:  %11\n"
        "Guards Assigned:  %12\n"
        "Staffing Status:  %13\n"
        "================================================\n"
        "Today's Attendance:\n"
        "  Present:        %14\n"
        "  Absent:         %15\n"
        "================================================\n"
        "Assigned Guards:\n%16"
        "================================================\n"
        "Instructions:\n%17"
    ).arg(
        siteQuery.value("site_code").toString(),
        siteQuery.value("site_name").toString(),
        siteQuery.value("address").toString(),
        siteQuery.value("city").toString(),
        siteQuery.value("client_name").toString(),
        siteQuery.value("status").toString(),
        siteQuery.value("shift_morning").toString(),
        siteQuery.value("shift_afternoon").toString(),
        siteQuery.value("shift_night").toString(),
        siteQuery.value("supervisor_name").toString(),
        QString::number(required),
        QString::number(assigned),
        fillStatus,
        QString::number(presentToday),
        QString::number(absentToday),
        guardDetails,
        siteQuery.value("site_instructions").toString()
    );

    QMessageBox box(this);
    box.setWindowTitle("Site — " + siteQuery.value("site_name").toString());
    box.setText(info);
    box.setFont(QFont("Consolas", 10));
    box.exec();
}

void SiteWidget::filterSites(const QString& text)
{
    QString searchText = text.toLower();
    QString statusFilter = m_statusFilter->currentText();
    int clientFilterId = m_clientFilter->currentData().toInt();

    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool textMatch = searchText.isEmpty();
        bool statusMatch = (statusFilter == "All Status");
        bool clientMatch = (clientFilterId == 0);

        if (!textMatch) {
            for (int col = 1; col < m_table->columnCount(); ++col) {
                auto* item = m_table->item(row, col);
                if (item && item->text().toLower().contains(searchText)) {
                    textMatch = true;
                    break;
                }
            }
        }

        if (!statusMatch) {
            auto* statusItem = m_table->item(row, 11);
            if (statusItem) {
                statusMatch = (statusItem->text() == statusFilter);
            }
        }

        if (!clientMatch) {
            auto& db = DatabaseManager::instance();
            int siteId = m_table->item(row, 0)->text().toInt();
            QSqlQuery q = db.execute(
                "SELECT client_id FROM Sites WHERE id = :id", {{":id", siteId}}
            );
            if (q.next()) {
                clientMatch = (q.value("client_id").toInt() == clientFilterId);
            }
        }

        m_table->setRowHidden(row, !(textMatch && statusMatch && clientMatch));
    }
}
