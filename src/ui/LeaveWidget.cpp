#include "LeaveWidget.h"
#include "LeaveDialog.h"
#include "database/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDate>
#include <QCoreApplication>
#include <QDir>
#include <QFrame>

LeaveWidget::LeaveWidget(QWidget* parent, int userId)
    : QWidget(parent), m_userId(userId)
{
    buildUI();
    loadLeaves();
    loadBalanceTab();
}

void LeaveWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Leave Management");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    auto* subtitle = new QLabel("Apply, approve and track guard leave balances");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    auto* tabWidget = new QTabWidget;
    tabWidget->addTab(buildRequestsTab(), "Leave Requests");
    tabWidget->addTab(buildBalanceTab(), "Leave Balances");

    connect(tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 0) loadLeaves();
        if (index == 1) loadBalanceTab();
    });

    mainLayout->addWidget(tabWidget, 1);
}

QWidget* LeaveWidget::buildRequestsTab()
{
    auto* tab = new QWidget;
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    // Header row
    auto* headerRow = new QHBoxLayout;
    m_countLabel = new QLabel("0 requests");
    m_countLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    headerRow->addWidget(m_countLabel);
    headerRow->addStretch();

    m_applyBtn = new QPushButton("+ Apply Leave");
    m_applyBtn->setObjectName("PrimaryButton");
    m_applyBtn->setFixedSize(140, 36);
    m_applyBtn->setCursor(Qt::PointingHandCursor);
    connect(m_applyBtn, &QPushButton::clicked, this, &LeaveWidget::applyLeave);
    headerRow->addWidget(m_applyBtn);

    layout->addLayout(headerRow);

    // Filter row
    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by guard name, code...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &LeaveWidget::filterLeaves);
    filterRow->addWidget(m_searchEdit, 1);

    m_statusFilter = new QComboBox;
    m_statusFilter->addItems({"All Status", "Pending", "Approved", "Rejected", "Cancelled"});
    m_statusFilter->setFixedWidth(140);
    connect(m_statusFilter, &QComboBox::currentTextChanged, this,
            [this](const QString&) { filterLeaves(m_searchEdit->text()); });
    filterRow->addWidget(m_statusFilter);

    m_typeFilter = new QComboBox;
    m_typeFilter->addItems({"All Types", "Casual", "Sick", "Earned", "Compensatory",
                            "Maternity", "Paternity", "Without Pay"});
    m_typeFilter->setFixedWidth(150);
    connect(m_typeFilter, &QComboBox::currentTextChanged, this,
            [this](const QString&) { filterLeaves(m_searchEdit->text()); });
    filterRow->addWidget(m_typeFilter);

    layout->addLayout(filterRow);

    // Action buttons
    auto* actionRow = new QHBoxLayout;
    actionRow->setSpacing(8);

    m_editBtn = new QPushButton("Edit");
    m_editBtn->setObjectName("SecondaryButton");
    m_editBtn->setFixedSize(70, 32);
    m_editBtn->setCursor(Qt::PointingHandCursor);
    connect(m_editBtn, &QPushButton::clicked, this, &LeaveWidget::editLeave);
    actionRow->addWidget(m_editBtn);

    m_approveBtn = new QPushButton("Approve");
    m_approveBtn->setObjectName("SecondaryButton");
    m_approveBtn->setFixedSize(90, 32);
    m_approveBtn->setCursor(Qt::PointingHandCursor);
    m_approveBtn->setStyleSheet(
        "QPushButton { background-color: #1A3A1A; color: #4ADE80; border: 1px solid #2A4A2A; "
        "border-radius: 6px; padding: 6px 16px; font-weight: 600; }"
        "QPushButton:hover { background-color: #2A4A2A; }");
    connect(m_approveBtn, &QPushButton::clicked, this, &LeaveWidget::approveLeave);
    actionRow->addWidget(m_approveBtn);

    m_rejectBtn = new QPushButton("Reject");
    m_rejectBtn->setObjectName("SecondaryButton");
    m_rejectBtn->setFixedSize(80, 32);
    m_rejectBtn->setCursor(Qt::PointingHandCursor);
    m_rejectBtn->setStyleSheet(
        "QPushButton { background-color: #3A1A1A; color: #E85454; border: 1px solid #5A2222; "
        "border-radius: 6px; padding: 6px 16px; font-weight: 600; }"
        "QPushButton:hover { background-color: #4A2020; }");
    connect(m_rejectBtn, &QPushButton::clicked, this, &LeaveWidget::rejectLeave);
    actionRow->addWidget(m_rejectBtn);

    m_deleteBtn = new QPushButton("Delete");
    m_deleteBtn->setObjectName("DangerButton");
    m_deleteBtn->setFixedSize(80, 32);
    m_deleteBtn->setCursor(Qt::PointingHandCursor);
    connect(m_deleteBtn, &QPushButton::clicked, this, &LeaveWidget::deleteLeave);
    actionRow->addWidget(m_deleteBtn);

    m_exportBtn = new QPushButton("Export CSV");
    m_exportBtn->setObjectName("SecondaryButton");
    m_exportBtn->setFixedSize(110, 32);
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    connect(m_exportBtn, &QPushButton::clicked, this, &LeaveWidget::exportCSV);
    actionRow->addWidget(m_exportBtn);

    actionRow->addStretch();
    layout->addLayout(actionRow);

    // Table
    m_table = new QTableWidget;
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(true);

    QStringList cols = {"ID", "Code", "Guard Name", "Leave Type", "Start", "End",
                        "Days", "Reason", "Status", "Applied On"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);

    m_table->setColumnWidth(1, 80);
    m_table->setColumnWidth(2, 160);
    m_table->setColumnWidth(3, 120);
    m_table->setColumnWidth(4, 95);
    m_table->setColumnWidth(5, 95);
    m_table->setColumnWidth(6, 55);
    m_table->setColumnWidth(7, 200);
    m_table->setColumnWidth(8, 90);
    m_table->horizontalHeader()->setStretchLastSection(true);

    connect(m_table, &QTableWidget::cellDoubleClicked, this, &LeaveWidget::editLeave);

    layout->addWidget(m_table, 1);
    return tab;
}

QWidget* LeaveWidget::buildBalanceTab()
{
    auto* tab = new QWidget;
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    auto* infoLabel = new QLabel(QString("Leave Balances for %1").arg(QDate::currentDate().year()));
    infoLabel->setStyleSheet("color: #D4B44C; font-size: 16px; font-weight: 700;");
    layout->addWidget(infoLabel);

    m_balanceTable = new QTableWidget;
    m_balanceTable->setAlternatingRowColors(true);
    m_balanceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_balanceTable->verticalHeader()->setVisible(false);
    m_balanceTable->setShowGrid(false);
    m_balanceTable->setSortingEnabled(true);

    QStringList cols = {"ID", "Code", "Guard Name",
                        "Casual Ent.", "Casual Used", "Casual Bal",
                        "Sick Ent.", "Sick Used", "Sick Bal",
                        "Earned Ent.", "Earned Used", "Earned Bal",
                        "Total Used", "Total Bal"};
    m_balanceTable->setColumnCount(cols.size());
    m_balanceTable->setHorizontalHeaderLabels(cols);
    m_balanceTable->setColumnHidden(0, true);

    m_balanceTable->setColumnWidth(1, 80);
    m_balanceTable->setColumnWidth(2, 160);
    for (int c = 3; c <= 13; ++c) m_balanceTable->setColumnWidth(c, 75);
    m_balanceTable->horizontalHeader()->setStretchLastSection(true);

    layout->addWidget(m_balanceTable, 1);
    return tab;
}

void LeaveWidget::loadLeaves()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT l.*, g.guard_code, g.full_name "
        "FROM LeaveRecord l JOIN Guards g ON l.guard_id = g.id "
        "ORDER BY l.created_at DESC"
    );

    m_table->setSortingEnabled(false);

    int count = 0;
    auto cq = db.execute("SELECT COUNT(*) FROM LeaveRecord");
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
        setItem(2, query.value("full_name").toString());
        setItem(3, query.value("leave_type").toString());
        setItem(4, query.value("start_date").toString());
        setItem(5, query.value("end_date").toString());

        auto* daysItem = new QTableWidgetItem(query.value("days").toString());
        daysItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 6, daysItem);

        setItem(7, query.value("reason").toString());

        // Status with color
        QString status = query.value("status").toString();
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "Approved")       statusItem->setForeground(QColor("#4ADE80"));
        else if (status == "Rejected")  statusItem->setForeground(QColor("#E85454"));
        else if (status == "Pending")   statusItem->setForeground(QColor("#FBBF24"));
        else if (status == "Cancelled") statusItem->setForeground(QColor("#6B7585"));
        m_table->setItem(row, 8, statusItem);

        setItem(9, query.value("created_at").toString());

        // Color code leave type
        auto* typeItem = m_table->item(row, 3);
        QString lt = query.value("leave_type").toString();
        if (lt == "Sick")       typeItem->setForeground(QColor("#60A5FA"));
        else if (lt == "Earned")      typeItem->setForeground(QColor("#A78BFA"));
        else if (lt == "Compensatory") typeItem->setForeground(QColor("#FB923C"));
        else if (lt == "Maternity" || lt == "Paternity") typeItem->setForeground(QColor("#F472B6"));
        else if (lt == "Without Pay")  typeItem->setForeground(QColor("#E85454"));

        row++;
    }

    m_table->setSortingEnabled(true);
    m_countLabel->setText(QString("%1 requests").arg(row));
}

void LeaveWidget::loadBalanceTab()
{
    auto& db = DatabaseManager::instance();
    int year = QDate::currentDate().year();

    // Get entitlements from settings
    auto getKey = [&](const QString& key, int def) -> int {
        auto sq = db.execute("SELECT value FROM Settings WHERE key = :k", {{":k", key}});
        return sq.next() ? sq.value("value").toInt() : def;
    };
    int casualEnt = getKey("casual_leave", 12);
    int sickEnt = getKey("sick_leave", 7);
    int earnedEnt = getKey("earned_leave", 15);

    // Get all active guards
    auto guards = db.execute(
        "SELECT id, guard_code, full_name FROM Guards "
        "WHERE status = 'Active' ORDER BY full_name"
    );

    // Get used leaves for this year
    auto usedQ = db.execute(
        "SELECT guard_id, leave_type, SUM(days) AS used_days "
        "FROM LeaveRecord WHERE status = 'Approved' "
        "AND strftime('%Y', start_date) = :year "
        "GROUP BY guard_id, leave_type",
        {{":year", QString::number(year)}}
    );

    // guard_id -> leave_type -> days
    QMap<int, QMap<QString, int>> usedMap;
    while (usedQ.next()) {
        int gid = usedQ.value("guard_id").toInt();
        QString lt = usedQ.value("leave_type").toString();
        usedMap[gid][lt] = usedQ.value("used_days").toInt();
    }

    // Count guards
    int gCount = 0;
    auto gc = db.execute("SELECT COUNT(*) FROM Guards WHERE status = 'Active'");
    if (gc.next()) gCount = gc.value(0).toInt();
    m_balanceTable->setRowCount(gCount);

    m_balanceTable->setSortingEnabled(false);
    int row = 0;
    guards = db.execute(
        "SELECT id, guard_code, full_name FROM Guards "
        "WHERE status = 'Active' ORDER BY full_name"
    );

    while (guards.next()) {
        int gid = guards.value("id").toInt();

        auto setCell = [&](int col, const QString& text,
                           const QColor& fg = QColor(), const QColor& bg = QColor()) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            if (fg.isValid()) item->setForeground(QBrush(fg));
            if (bg.isValid()) item->setBackground(QBrush(bg));
            m_balanceTable->setItem(row, col, item);
        };

        auto* codeItem = new QTableWidgetItem(guards.value("guard_code").toString());
        codeItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_balanceTable->setItem(row, 0, new QTableWidgetItem(QString::number(gid)));
        m_balanceTable->setItem(row, 1, codeItem);

        auto* nameItem = new QTableWidgetItem(guards.value("full_name").toString());
        nameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_balanceTable->setItem(row, 2, nameItem);

        int uCasual = usedMap[gid]["Casual"];
        int uSick = usedMap[gid]["Sick"];
        int uEarned = usedMap[gid]["Earned"];

        int bCasual = casualEnt - uCasual;
        int bSick = sickEnt - uSick;
        int bEarned = earnedEnt - uEarned;

        auto balColor = [](int bal) -> QColor {
            if (bal <= 0) return QColor("#E85454");
            if (bal <= 2) return QColor("#FBBF24");
            return QColor("#4ADE80");
        };

        // Casual
        setCell(3, QString::number(casualEnt));
        setCell(4, QString::number(uCasual), uCasual > 0 ? QColor("#FBBF24") : QColor());
        setCell(5, QString::number(bCasual), balColor(bCasual));

        // Sick
        setCell(6, QString::number(sickEnt));
        setCell(7, QString::number(uSick), uSick > 0 ? QColor("#FBBF24") : QColor());
        setCell(8, QString::number(bSick), balColor(bSick));

        // Earned
        setCell(9, QString::number(earnedEnt));
        setCell(10, QString::number(uEarned), uEarned > 0 ? QColor("#FBBF24") : QColor());
        setCell(11, QString::number(bEarned), balColor(bEarned));

        // Totals
        int totalUsed = uCasual + uSick + uEarned;
        int totalBal = bCasual + bSick + bEarned;
        setCell(12, QString::number(totalUsed));
        setCell(13, QString::number(totalBal));

        row++;
    }

    m_balanceTable->setSortingEnabled(true);
}

void LeaveWidget::refresh()
{
    loadLeaves();
    loadBalanceTab();
}

int selectedLeaveId(QTableWidget* table) {
    auto items = table->selectedItems();
    if (items.isEmpty()) return -1;
    return table->item(items.first()->row(), 0)->text().toInt();
}

void LeaveWidget::applyLeave()
{
    LeaveDialog dlg(this, -1, m_userId);
    if (dlg.exec() == QDialog::Accepted) {
        loadLeaves();
        loadBalanceTab();
    }
}

void LeaveWidget::editLeave()
{
    int id = selectedLeaveId(m_table);
    if (id < 0) {
        QMessageBox::information(this, "No Selection", "Select a leave request to edit.");
        return;
    }
    LeaveDialog dlg(this, id, m_userId);
    if (dlg.exec() == QDialog::Accepted) {
        loadLeaves();
        loadBalanceTab();
    }
}

void LeaveWidget::approveLeave()
{
    int id = selectedLeaveId(m_table);
    if (id < 0) {
        QMessageBox::information(this, "No Selection", "Select a leave request to approve.");
        return;
    }

    int row = m_table->currentRow();
    QString guard = m_table->item(row, 2)->text();
    QString status = m_table->item(row, 8)->text();

    if (status == "Approved") {
        QMessageBox::information(this, "Already Approved", "This leave is already approved.");
        return;
    }

    auto result = QMessageBox::question(this, "Approve Leave",
        QString("Approve leave for \"%1\"?").arg(guard),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        QVariantMap params;
        params[":id"] = id;
        params[":by"] = m_userId > 0 ? m_userId : QVariant();

        db.executeNonQuery(
            "UPDATE LeaveRecord SET status = 'Approved', approved_by = :by WHERE id = :id",
            params
        );

        // Also mark attendance as Leave for those days
        auto lq = db.execute("SELECT guard_id, start_date, end_date FROM LeaveRecord WHERE id = :id",
                             {{":id", id}});
        if (lq.next()) {
            int gid = lq.value("guard_id").toInt();
            QDate start = QDate::fromString(lq.value("start_date").toString(), "yyyy-MM-dd");
            QDate end = QDate::fromString(lq.value("end_date").toString(), "yyyy-MM-dd");
            QDate d = start;
            while (d <= end) {
                int siteId = 0;
                auto sq = db.execute("SELECT site_id FROM Guards WHERE id = :id", {{":id", gid}});
                if (sq.next()) siteId = sq.value("site_id").toInt();

                if (siteId > 0) {
                    db.executeNonQuery(
                        "INSERT INTO Attendance (guard_id, site_id, date, status, notes) "
                        "VALUES (:gid, :sid, :date, 'Leave', 'Auto-marked from approved leave') "
                        "ON CONFLICT(guard_id, date) DO UPDATE SET status = 'Leave', "
                        "notes = 'Auto-marked from approved leave'",
                        {{":gid", gid}, {":sid", siteId}, {":date", d.toString("yyyy-MM-dd")}}
                    );
                }
                d = d.addDays(1);
            }
        }

        loadLeaves();
        loadBalanceTab();
    }
}

void LeaveWidget::rejectLeave()
{
    int id = selectedLeaveId(m_table);
    if (id < 0) {
        QMessageBox::information(this, "No Selection", "Select a leave request to reject.");
        return;
    }

    int row = m_table->currentRow();
    QString guard = m_table->item(row, 2)->text();

    auto result = QMessageBox::question(this, "Reject Leave",
        QString("Reject leave for \"%1\"?").arg(guard),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("UPDATE LeaveRecord SET status = 'Rejected' WHERE id = :id", {{":id", id}});
        loadLeaves();
    }
}

void LeaveWidget::deleteLeave()
{
    int id = selectedLeaveId(m_table);
    if (id < 0) {
        QMessageBox::information(this, "No Selection", "Select a leave request to delete.");
        return;
    }

    int row = m_table->currentRow();
    QString guard = m_table->item(row, 2)->text();

    auto result = QMessageBox::question(this, "Delete",
        QString("Delete leave request for \"%1\"?").arg(guard),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("DELETE FROM LeaveRecord WHERE id = :id", {{":id", id}});
        loadLeaves();
        loadBalanceTab();
    }
}

void LeaveWidget::filterLeaves(const QString& text)
{
    QString searchText = text.toLower();
    QString statusFilter = m_statusFilter->currentText();
    QString typeFilter = m_typeFilter->currentText();

    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool textMatch = searchText.isEmpty();
        bool statusMatch = (statusFilter == "All Status");
        bool typeMatch = (typeFilter == "All Types");

        if (!textMatch) {
            for (int col = 1; col <= 7; ++col) {
                auto* item = m_table->item(row, col);
                if (item && item->text().toLower().contains(searchText)) {
                    textMatch = true; break;
                }
            }
        }

        if (!statusMatch) {
            auto* item = m_table->item(row, 8);
            if (item) statusMatch = (item->text() == statusFilter);
        }

        if (!typeMatch) {
            auto* item = m_table->item(row, 3);
            if (item) typeMatch = (item->text() == typeFilter);
        }

        m_table->setRowHidden(row, !(textMatch && statusMatch && typeMatch));
    }
}

void LeaveWidget::exportCSV()
{
    if (m_table->rowCount() == 0) {
        QMessageBox::information(this, "No Data", "No leave data to export.");
        return;
    }

    QDir().mkpath(QCoreApplication::applicationDirPath() + "/reports");
    QString filePath = QFileDialog::getSaveFileName(
        this, "Export Leave Report",
        QCoreApplication::applicationDirPath() + "/reports/leave_report.csv",
        "CSV Files (*.csv);;All Files (*)"
    );
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file.");
        return;
    }

    QTextStream out(&file);

    QStringList headerParts;
    for (int col = 0; col < m_table->columnCount(); ++col) {
        auto* h = m_table->horizontalHeaderItem(col);
        headerParts << (h ? h->text() : "");
    }
    out << headerParts.join(",") << "\n";

    for (int row = 0; row < m_table->rowCount(); ++row) {
        if (m_table->isRowHidden(row)) continue;
        QStringList rowParts;
        for (int col = 0; col < m_table->columnCount(); ++col) {
            auto* item = m_table->item(row, col);
            QString text = item ? item->text() : "";
            if (text.contains(',') || text.contains('"')) {
                text = "\"" + text.replace("\"", "\"\"") + "\"";
            }
            rowParts << text;
        }
        out << rowParts.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, "Export Successful",
        QString("Leave report exported to:\n\n%1").arg(filePath));
}
