#include "DutyWidget.h"
#include "DutyDialog.h"
#include "database/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QFrame>
#include <QDate>

DutyWidget::DutyWidget(QWidget* parent)
    : QWidget(parent)
{
    buildUI();
    loadDuties();
}

void DutyWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Duty Allocation");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    auto* subtitle = new QLabel("Assign guards, manage rosters, handle transfers and emergencies");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);

    mainLayout->addSpacing(8);

    auto* tabWidget = new QTabWidget;

    // Tab 1: All Assignments
    auto* assignmentTab = new QWidget;
    auto* assignLayout = new QVBoxLayout(assignmentTab);
    assignLayout->setContentsMargins(12, 12, 12, 12);
    assignLayout->setSpacing(12);

    // Header row
    auto* headerRow = new QHBoxLayout;
    m_countLabel = new QLabel("0 assignments");
    m_countLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    headerRow->addWidget(m_countLabel);
    headerRow->addStretch();

    m_transferBtn = new QPushButton("Quick Transfer");
    m_transferBtn->setObjectName("SecondaryButton");
    m_transferBtn->setFixedSize(140, 36);
    m_transferBtn->setCursor(Qt::PointingHandCursor);
    connect(m_transferBtn, &QPushButton::clicked, this, &DutyWidget::transferGuard);
    headerRow->addWidget(m_transferBtn);

    m_addBtn = new QPushButton("+ New Assignment");
    m_addBtn->setObjectName("PrimaryButton");
    m_addBtn->setFixedSize(170, 36);
    m_addBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addBtn, &QPushButton::clicked, this, &DutyWidget::addDuty);
    headerRow->addWidget(m_addBtn);

    assignLayout->addLayout(headerRow);

    // Filter row
    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by guard name, code, site...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &DutyWidget::filterDuties);
    filterRow->addWidget(m_searchEdit, 1);

    m_siteFilter = new QComboBox;
    m_siteFilter->addItem("All Sites", 0);
    m_siteFilter->setFixedWidth(200);
    connect(m_siteFilter, &QComboBox::currentTextChanged, this,
            [this](const QString&) { filterDuties(m_searchEdit->text()); });
    filterRow->addWidget(m_siteFilter);

    m_shiftFilter = new QComboBox;
    m_shiftFilter->addItems({"All Shifts", "Morning", "Afternoon", "Night", "General"});
    m_shiftFilter->setFixedWidth(130);
    connect(m_shiftFilter, &QComboBox::currentTextChanged, this,
            [this](const QString&) { filterDuties(m_searchEdit->text()); });
    filterRow->addWidget(m_shiftFilter);

    m_typeFilter = new QComboBox;
    m_typeFilter->addItems({
        "All Types", "Regular", "Temporary Transfer", "Emergency",
        "Overtime", "Replacement", "Training Duty"
    });
    m_typeFilter->setFixedWidth(170);
    connect(m_typeFilter, &QComboBox::currentTextChanged, this,
            [this](const QString&) { filterDuties(m_searchEdit->text()); });
    filterRow->addWidget(m_typeFilter);

    assignLayout->addLayout(filterRow);

    // Action buttons
    auto* actionRow = new QHBoxLayout;
    actionRow->setSpacing(8);

    m_editBtn = new QPushButton("Edit");
    m_editBtn->setObjectName("SecondaryButton");
    m_editBtn->setFixedSize(80, 32);
    m_editBtn->setCursor(Qt::PointingHandCursor);
    connect(m_editBtn, &QPushButton::clicked, this, &DutyWidget::editDuty);

    m_viewBtn = new QPushButton("View Roster");
    m_viewBtn->setObjectName("SecondaryButton");
    m_viewBtn->setFixedSize(120, 32);
    m_viewBtn->setCursor(Qt::PointingHandCursor);
    connect(m_viewBtn, &QPushButton::clicked, this, &DutyWidget::viewRoster);

    m_deleteBtn = new QPushButton("Remove");
    m_deleteBtn->setObjectName("DangerButton");
    m_deleteBtn->setFixedSize(90, 32);
    m_deleteBtn->setCursor(Qt::PointingHandCursor);
    connect(m_deleteBtn, &QPushButton::clicked, this, &DutyWidget::deleteDuty);

    actionRow->addWidget(m_editBtn);
    actionRow->addWidget(m_viewBtn);
    actionRow->addWidget(m_deleteBtn);
    actionRow->addStretch();

    assignLayout->addLayout(actionRow);

    // Table
    m_table = new QTableWidget;
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(true);

    QStringList cols = {
        "ID", "Guard Code", "Guard Name", "Site", "Client",
        "Shift", "Type", "Start", "End", "Permanent", "Status"
    };
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);

    m_table->setColumnWidth(1, 90);
    m_table->setColumnWidth(2, 160);
    m_table->setColumnWidth(3, 160);
    m_table->setColumnWidth(4, 140);
    m_table->setColumnWidth(5, 90);
    m_table->setColumnWidth(6, 130);
    m_table->setColumnWidth(7, 95);
    m_table->setColumnWidth(8, 95);
    m_table->setColumnWidth(9, 85);
    m_table->horizontalHeader()->setStretchLastSection(true);

    connect(m_table, &QTableWidget::cellDoubleClicked, this, &DutyWidget::editDuty);

    assignLayout->addWidget(m_table, 1);

    tabWidget->addTab(assignmentTab, "All Assignments");

    // Tab 2: Weekly Roster View
    tabWidget->addTab(buildRosterTab(), "Weekly Roster");

    tabWidget->addTab(buildRosterTab(), "Weekly Roster");

    mainLayout->addWidget(tabWidget, 1);
}

QWidget* DutyWidget::buildRosterTab()
{
    auto* tab = new QWidget;
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    // Controls
    auto* controlRow = new QHBoxLayout;
    controlRow->setSpacing(12);

    auto* weekLbl = new QLabel("Week Starting:");
    weekLbl->setStyleSheet("color: #8B95A5; font-weight: 600;");
    controlRow->addWidget(weekLbl);

    m_rosterDate = new QDateEdit;
    m_rosterDate->setCalendarPopup(true);
    m_rosterDate->setDisplayFormat("yyyy-MM-dd");
    // Set to start of current week (Monday)
    QDate today = QDate::currentDate();
    QDate weekStart = today.addDays(-(today.dayOfWeek() - 1));
    m_rosterDate->setDate(weekStart);
    m_rosterDate->setFixedWidth(140);
    connect(m_rosterDate, &QDateEdit::dateChanged, this, &DutyWidget::viewRoster);
    layout->addWidget(m_rosterDate);

    controlRow->addSpacing(16);

    auto* siteLbl = new QLabel("Site:");
    siteLbl->setStyleSheet("color: #8B95A5; font-weight: 600;");
    controlRow->addWidget(siteLbl);

    m_rosterSite = new QComboBox;
    m_rosterSite->addItem("All Sites", 0);
    m_rosterSite->setFixedWidth(200);
    connect(m_rosterSite, &QComboBox::currentTextChanged, this,
            [this](const QString&) { viewRoster(); });
    controlRow->addWidget(m_rosterSite);

    controlRow->addStretch();
    layout->addLayout(controlRow);

    // Roster table
    m_rosterTable = new QTableWidget;
    m_rosterTable->setAlternatingRowColors(true);
    m_rosterTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_rosterTable->verticalHeader()->setVisible(false);
    m_rosterTable->setShowGrid(true);
    m_rosterTable->setGridStyle(Qt::SolidLine);
    layout->addWidget(m_rosterTable, 1);

    return tab;
}

void DutyWidget::loadDuties()
{
    auto& db = DatabaseManager::instance();

    // Populate site filter
    m_siteFilter->blockSignals(true);
    int currentSite = m_siteFilter->currentData().toInt();
    m_siteFilter->clear();
    m_siteFilter->addItem("All Sites", 0);
    auto sites = db.execute("SELECT id, site_name FROM Sites ORDER BY site_name");
    while (sites.next()) {
        m_siteFilter->addItem(sites.value("site_name").toString(), sites.value("id").toInt());
    }
    for (int i = 0; i < m_siteFilter->count(); ++i) {
        if (m_siteFilter->itemData(i).toInt() == currentSite) {
            m_siteFilter->setCurrentIndex(i);
            break;
        }
    }
    m_siteFilter->blockSignals(false);

    // Populate roster site filter
    m_rosterSite->blockSignals(true);
    int currentRosterSite = m_rosterSite->currentData().toInt();
    m_rosterSite->clear();
    m_rosterSite->addItem("All Sites", 0);
    auto sites2 = db.execute("SELECT id, site_name FROM Sites ORDER BY site_name");
    while (sites2.next()) {
        m_rosterSite->addItem(sites2.value("site_name").toString(), sites2.value("id").toInt());
    }
    for (int i = 0; i < m_rosterSite->count(); ++i) {
        if (m_rosterSite->itemData(i).toInt() == currentRosterSite) {
            m_rosterSite->setCurrentIndex(i);
            break;
        }
    }
    m_rosterSite->blockSignals(false);

    // Load all duty assignments
    auto query = db.execute(
        "SELECT d.*, "
        "g.guard_code, g.full_name AS guard_name, "
        "s.site_name, c.company_name AS client_name "
        "FROM Duty d "
        "JOIN Guards g ON d.guard_id = g.id "
        "JOIN Sites s ON d.site_id = s.id "
        "LEFT JOIN Clients c ON s.client_id = c.id "
        "ORDER BY g.full_name"
    );

    m_table->setSortingEnabled(false);

    int count = 0;
    QSqlQuery cq = db.execute("SELECT COUNT(*) FROM Duty");
    if (cq.next()) count = cq.value(0).toInt();
    m_table->setRowCount(count);

    int row = 0;
    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        setItem(1, query.value("guard_code").toString());
        setItem(2, query.value("guard_name").toString());
        setItem(3, query.value("site_name").toString());
        setItem(4, query.value("client_name").toString());
        setItem(5, query.value("shift").toString());
        setItem(6, query.value("notes").toString().contains("Transfer")
                     ? "Temporary Transfer" : "Regular");
        setItem(7, query.value("start_date").toString());
        setItem(8, query.value("end_date").toString());
        setItem(9, query.value("is_permanent").toBool() ? "Yes" : "No");

        // Status based on dates
        QDate today = QDate::currentDate();
        QDate start = QDate::fromString(query.value("start_date").toString(), "yyyy-MM-dd");
        QDate end   = QDate::fromString(query.value("end_date").toString(), "yyyy-MM-dd");
        bool perm   = query.value("is_permanent").toBool();

        QString status;
        QColor statusColor;
        if (start > today) {
            status = "Upcoming";
            statusColor = QColor("#FBBF24");
        } else if (perm || !end.isValid() || end >= today) {
            status = "Active";
            statusColor = QColor("#4ADE80");
        } else {
            status = "Expired";
            statusColor = QColor("#E85454");
        }

        setItem(10, status);
        m_table->item(row, 10)->setForeground(statusColor);

        // Color-code shift
        auto* shiftItem = m_table->item(row, 5);
        QString shift = query.value("shift").toString();
        if (shift == "Morning")       shiftItem->setForeground(QColor("#FBBF24"));
        else if (shift == "Afternoon") shiftItem->setForeground(QColor("#FB923C"));
        else if (shift == "Night")     shiftItem->setForeground(QColor("#A78BFA"));
        else if (shift == "General")   shiftItem->setForeground(QColor("#60A5FA"));

        row++;
    }

    m_table->setSortingEnabled(true);
    m_countLabel->setText(QString("%1 assignments").arg(row));
}

void DutyWidget::refresh()
{
    loadDuties();
}

int DutyWidget::selectedDutyId() const
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) return -1;
    return m_table->item(items.first()->row(), 0)->text().toInt();
}

void DutyWidget::addDuty()
{
    DutyDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        loadDuties();
    }
}

void DutyWidget::editDuty()
{
    int id = selectedDutyId();
    if (id < 0) {
        QMessageBox::information(this, "No Selection", "Please select an assignment to edit.");
        return;
    }
    DutyDialog dlg(this, id);
    if (dlg.exec() == QDialog::Accepted) {
        loadDuties();
    }
}

void DutyWidget::deleteDuty()
{
    int id = selectedDutyId();
    if (id < 0) {
        QMessageBox::information(this, "No Selection", "Please select an assignment to remove.");
        return;
    }

    int row = m_table->currentRow();
    QString guard = m_table->item(row, 2)->text();
    QString site  = m_table->item(row, 3)->text();

    auto result = QMessageBox::question(
        this, "Confirm Remove",
        QString("Remove duty assignment for \"%1\" at \"%2\"?")
            .arg(guard, site),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        if (db.executeNonQuery("DELETE FROM Duty WHERE id = :id", {{":id", id}})) {
            loadDuties();
        }
    }
}

void DutyWidget::transferGuard()
{
    int id = selectedDutyId();
    if (id < 0) {
        QMessageBox::information(this, "No Selection",
            "Select a current assignment first, then click Quick Transfer.\n\n"
            "Or use '+ New Assignment' to create a new duty from scratch.");
        return;
    }

    int row = m_table->currentRow();
    QString guardName = m_table->item(row, 2)->text();
    QString currentSite = m_table->item(row, 3)->text();

    QMessageBox::information(this, "Transfer Guard",
        QString("To transfer \"%1\" from \"%2\":\n\n"
                "1. Edit the current assignment and set an end date\n"
                "2. Create a new assignment for the new site\n\n"
                "This preserves the duty history.").arg(guardName, currentSite));

    DutyDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        loadDuties();
    }
}

void DutyWidget::viewRoster()
{
    auto& db = DatabaseManager::instance();

    QDate weekStart = m_rosterDate->date();
    int siteId = m_rosterSite->currentData().toInt();

    // 7 days of the week
    QStringList dayNames = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    QDate weekEnd = weekStart.addDays(6);

    // Get all active duties overlapping this week
    QString sql =
        "SELECT d.*, g.guard_code, g.full_name, s.site_name "
        "FROM Duty d "
        "JOIN Guards g ON d.guard_id = g.id "
        "JOIN Sites s ON d.site_id = s.id "
        "WHERE d.start_date <= :weekEnd "
        "AND (d.end_date >= :weekStart OR d.is_permanent = 1) ";
    QVariantMap params;
    params[":weekStart"] = weekStart.toString("yyyy-MM-dd");
    params[":weekEnd"]   = weekEnd.toString("yyyy-MM-dd");

    if (siteId > 0) {
        sql += "AND d.site_id = :siteId ";
        params[":siteId"] = siteId;
    }
    sql += "ORDER BY s.site_name, g.full_name";

    auto query = db.execute(sql, params);

    // Collect roster data
    struct RosterEntry {
        QString guardCode;
        QString guardName;
        QString site;
        QString shift;
    };
    QList<RosterEntry> entries;
    while (query.next()) {
        entries.append({
            query.value("guard_code").toString(),
            query.value("full_name").toString(),
            query.value("site_name").toString(),
            query.value("shift").toString()
        });
    }

    // Build roster table
    int totalCols = 4 + 7; // Code, Name, Site, Shift + 7 days
    m_rosterTable->clear();
    m_rosterTable->setColumnCount(totalCols);
    m_rosterTable->setRowCount(entries.size());

    QStringList headers;
    headers << "Code" << "Guard Name" << "Site" << "Shift";
    for (int d = 0; d < 7; ++d) {
        QDate dayDate = weekStart.addDays(d);
        headers << dayDate.toString("dd\n" + dayNames[d]);
    }
    m_rosterTable->setHorizontalHeaderLabels(headers);

    m_rosterTable->setColumnWidth(0, 80);
    m_rosterTable->setColumnWidth(1, 160);
    m_rosterTable->setColumnWidth(2, 150);
    m_rosterTable->setColumnWidth(3, 90);
    for (int d = 0; d < 7; ++d) {
        m_rosterTable->setColumnWidth(4 + d, 65);
    }

    for (int row = 0; row < entries.size(); ++row) {
        const auto& e = entries[row];

        auto setCell = [&](int col, const QString& text,
                           const QColor& fg = QColor(),
                           const QColor& bg = QColor()) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            if (fg.isValid()) item->setForeground(QBrush(fg));
            if (bg.isValid()) item->setBackground(QBrush(bg));
            m_rosterTable->setItem(row, col, item);
        };

        setCell(0, e.guardCode);
        setCell(1, e.guardName);

        auto* nameItem = m_rosterTable->item(row, 1);
        nameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        setCell(2, e.site);
        auto* siteItem = m_rosterTable->item(row, 2);
        siteItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        // Shift color
        QColor shiftFg = QColor("#8B95A5");
        if (e.shift == "Morning")       shiftFg = QColor("#FBBF24");
        else if (e.shift == "Afternoon") shiftFg = QColor("#FB923C");
        else if (e.shift == "Night")     shiftFg = QColor("#A78BFA");
        else if (e.shift == "General")   shiftFg = QColor("#60A5FA");
        setCell(3, e.shift, shiftFg);

        // Day cells - fill with shift indicator
        for (int d = 0; d < 7; ++d) {
            QDate dayDate = weekStart.addDays(d);
            QColor dayBg = QColor("#1A2233");
            QColor dayFg = shiftFg;

            // Check attendance for this guard on this day
            auto att = db.execute(
                "SELECT status FROM Attendance "
                "WHERE guard_id = (SELECT id FROM Guards WHERE guard_code = :code) "
                "AND date = :date",
                {{":code", e.guardCode}, {":date", dayDate.toString("yyyy-MM-dd")}}
            );

            if (att.next()) {
                QString status = att.value("status").toString();
                if (status == "Present") {
                    dayBg = QColor("#1A3A1A"); dayFg = QColor("#4ADE80");
                    setCell(4 + d, "P", dayFg, dayBg);
                } else if (status == "Absent") {
                    dayBg = QColor("#3A1A1A"); dayFg = QColor("#E85454");
                    setCell(4 + d, "A", dayFg, dayBg);
                } else if (status == "Leave") {
                    dayBg = QColor("#1A1A3A"); dayFg = QColor("#60A5FA");
                    setCell(4 + d, "L", dayFg, dayBg);
                } else if (status == "Half Day") {
                    dayBg = QColor("#3A3A1A"); dayFg = QColor("#FBBF24");
                    setCell(4 + d, "H", dayFg, dayBg);
                } else {
                    setCell(4 + d, status.left(1).toUpper(), dayFg, dayBg);
                }
            } else {
                // No attendance - show shift indicator
                QString shiftLetter = e.shift.left(1);
                setCell(4 + d, shiftLetter, shiftFg, dayBg);
            }

            // Highlight today
            if (dayDate == QDate::currentDate()) {
                auto* todayItem = m_rosterTable->item(row, 4 + d);
                if (todayItem) {
                    QFont f = todayItem->font();
                    f.setBold(true);
                    todayItem->setFont(f);
                }
            }

            // Weekend background
            if (dayDate.dayOfWeek() >= 6) {
                auto* weekendItem = m_rosterTable->item(row, 4 + d);
                if (weekendItem && !weekendItem->background().color().isValid()) {
                    weekendItem->setBackground(QBrush(QColor("#151A22")));
                }
            }
        }
    }
}

void DutyWidget::filterDuties(const QString& text)
{
    QString searchText = text.toLower();
    QString shiftFilter = m_shiftFilter->currentText();
    int siteFilterId = m_siteFilter->currentData().toInt();

    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool textMatch = searchText.isEmpty();
        bool shiftMatch = (shiftFilter == "All Shifts");
        bool siteMatch = (siteFilterId == 0);

        if (!textMatch) {
            for (int col = 1; col < m_table->columnCount(); ++col) {
                auto* item = m_table->item(row, col);
                if (item && item->text().toLower().contains(searchText)) {
                    textMatch = true;
                    break;
                }
            }
        }

        if (!shiftMatch) {
            auto* shiftItem = m_table->item(row, 5);
            if (shiftItem) shiftMatch = (shiftItem->text() == shiftFilter);
        }

        m_table->setRowHidden(row, !(textMatch && shiftMatch && siteMatch));
    }
}
