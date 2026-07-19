#include "AttendanceWidget.h"
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
#include <QFrame>
#include <QDir>

AttendanceWidget::AttendanceWidget(QWidget* parent, int userId)
    : QWidget(parent), m_userId(userId)
{
    buildUI();
}

void AttendanceWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Attendance Management");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    auto* subtitle = new QLabel("Mark daily attendance and view monthly registers");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    auto* tabWidget = new QTabWidget;
    tabWidget->addTab(buildDailyTab(), "Daily Attendance");
    tabWidget->addTab(buildMonthlyTab(), "Monthly Register");

    connect(tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 0) loadDailyAttendance();
        if (index == 1) loadMonthlyRegister();
    });

    mainLayout->addWidget(tabWidget, 1);
}

QWidget* AttendanceWidget::buildDailyTab()
{
    auto* tab = new QWidget;
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    // Controls row
    auto* controlRow = new QHBoxLayout;
    controlRow->setSpacing(12);

    auto* dateLbl = new QLabel("Date:");
    dateLbl->setStyleSheet("color: #8B95A5; font-weight: 600;");
    controlRow->addWidget(dateLbl);

    m_dailyDate = new QDateEdit;
    m_dailyDate->setCalendarPopup(true);
    m_dailyDate->setDisplayFormat("yyyy-MM-dd");
    m_dailyDate->setDate(QDate::currentDate());
    m_dailyDate->setFixedWidth(140);
    connect(m_dailyDate, &QDateEdit::dateChanged, this, &AttendanceWidget::loadDailyAttendance);
    controlRow->addWidget(m_dailyDate);

    auto* todayBtn = new QPushButton("Today");
    todayBtn->setObjectName("SecondaryButton");
    todayBtn->setFixedSize(70, 36);
    todayBtn->setCursor(Qt::PointingHandCursor);
    connect(todayBtn, &QPushButton::clicked, this, [this]() {
        m_dailyDate->setDate(QDate::currentDate());
    });
    controlRow->addWidget(todayBtn);
    controlRow->addSpacing(16);

    auto* siteLbl = new QLabel("Site:");
    siteLbl->setStyleSheet("color: #8B95A5; font-weight: 600;");
    controlRow->addWidget(siteLbl);

    m_dailySiteCombo = new QComboBox;
    m_dailySiteCombo->setFixedWidth(220);
    connect(m_dailySiteCombo, &QComboBox::currentTextChanged, this,
            [this](const QString&) { loadDailyAttendance(); });
    controlRow->addWidget(m_dailySiteCombo);

    m_shiftLabel = new QLabel;
    m_shiftLabel->setStyleSheet("color: #555E6B; font-size: 12px; font-style: italic;");
    controlRow->addWidget(m_shiftLabel);
    controlRow->addStretch();
    layout->addLayout(controlRow);

    // Stats and action row
    auto* actionRow = new QHBoxLayout;
    actionRow->setSpacing(12);

    m_statsLabel = new QLabel;
    m_statsLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    actionRow->addWidget(m_statsLabel);
    actionRow->addStretch();

    m_clearBtn = new QPushButton("Clear All");
    m_clearBtn->setObjectName("SecondaryButton");
    m_clearBtn->setFixedSize(100, 36);
    m_clearBtn->setCursor(Qt::PointingHandCursor);
    connect(m_clearBtn, &QPushButton::clicked, this, &AttendanceWidget::clearAllStatus);
    actionRow->addWidget(m_clearBtn);

    m_markAllBtn = new QPushButton("Mark All Present");
    m_markAllBtn->setObjectName("SecondaryButton");
    m_markAllBtn->setFixedSize(160, 36);
    m_markAllBtn->setCursor(Qt::PointingHandCursor);
    connect(m_markAllBtn, &QPushButton::clicked, this, &AttendanceWidget::markAllPresent);
    actionRow->addWidget(m_markAllBtn);

    m_saveBtn = new QPushButton("Save Attendance");
    m_saveBtn->setObjectName("PrimaryButton");
    m_saveBtn->setFixedSize(170, 36);
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    connect(m_saveBtn, &QPushButton::clicked, this, &AttendanceWidget::saveDailyAttendance);
    actionRow->addWidget(m_saveBtn);
    layout->addLayout(actionRow);

    // Table
    m_dailyTable = new QTableWidget;
    m_dailyTable->setAlternatingRowColors(true);
    m_dailyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_dailyTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_dailyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_dailyTable->verticalHeader()->setVisible(false);
    m_dailyTable->setShowGrid(false);

    QStringList cols = {"Guard ID", "Code", "Guard Name", "Site", "Status", "Notes"};
    m_dailyTable->setColumnCount(cols.size());
    m_dailyTable->setHorizontalHeaderLabels(cols);
    m_dailyTable->setColumnHidden(0, true);
    m_dailyTable->setColumnWidth(1, 90);
    m_dailyTable->setColumnWidth(2, 200);
    m_dailyTable->setColumnWidth(3, 150);
    m_dailyTable->setColumnWidth(4, 180);
    m_dailyTable->horizontalHeader()->setStretchLastSection(true);

    layout->addWidget(m_dailyTable, 1);
    return tab;
}

void AttendanceWidget::loadDailyAttendance()
{
    auto& db = DatabaseManager::instance();

    // Populate site combo once
    if (m_dailySiteCombo->count() == 0) {
        m_dailySiteCombo->blockSignals(true);
        m_dailySiteCombo->addItem("All Sites", 0);
        auto sites = db.execute("SELECT id, site_name FROM Sites WHERE status = 'Active' ORDER BY site_name");
        while (sites.next()) {
            m_dailySiteCombo->addItem(sites.value("site_name").toString(), sites.value("id").toInt());
        }
        m_dailySiteCombo->blockSignals(false);
    }

    QDate date = m_dailyDate->date();
    int siteId = m_dailySiteCombo->currentData().toInt();
    QString dateStr = date.toString("yyyy-MM-dd");

    // Show shift timings
    if (siteId > 0) {
        auto sq = db.execute("SELECT shift_morning, shift_afternoon, shift_night FROM Sites WHERE id = :id", {{":id", siteId}});
        if (sq.next()) {
            m_shiftLabel->setText(QString("Shifts: %1 | %2 | %3")
                .arg(sq.value("shift_morning").toString(), sq.value("shift_afternoon").toString(), sq.value("shift_night").toString()));
        }
    } else {
        m_shiftLabel->setText("");
    }

    // Get guard list
    struct GuardRow { int id; QString code; QString name; QString site; };
    QList<GuardRow> guardRows;

    QString sql = "SELECT g.id, g.guard_code, g.full_name, COALESCE(s.site_name, 'Unassigned') AS site_name "
                  "FROM Guards g LEFT JOIN Sites s ON g.site_id = s.id "
                  "WHERE g.status = 'Active' AND g.site_id IS NOT NULL ";
    QVariantMap params;
    if (siteId > 0) {
        sql += "AND g.site_id = :siteId ";
        params[":siteId"] = siteId;
    }
    sql += "ORDER BY g.full_name";

    auto guards = db.execute(sql, params);
    while (guards.next()) {
        guardRows.append({guards.value("id").toInt(), guards.value("guard_code").toString(),
                          guards.value("full_name").toString(), guards.value("site_name").toString()});
    }

    // Get existing attendance
    auto att = db.execute("SELECT guard_id, status, notes FROM Attendance WHERE date = :date", {{":date", dateStr}});
    QMap<int, QPair<QString, QString>> attMap;
    while (att.next()) {
        int gid = att.value("guard_id").toInt();
        attMap[gid] = qMakePair(att.value("status").toString(), att.value("notes").toString());
    }

    // Populate table
    m_statusCombos.clear();
    m_notesEdits.clear();
    m_dailyTable->setRowCount(guardRows.size());

    for (int row = 0; row < guardRows.size(); ++row) {
        const auto& g = guardRows[row];

        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_dailyTable->setItem(row, col, item);
        };

        setItem(0, QString::number(g.id));
        setItem(1, g.code);
        setItem(2, g.name);
        setItem(3, g.site);

        // Status combo
        auto* combo = new QComboBox;
        combo->addItems({"", "Present", "Absent", "Half Day", "Leave", "Night Shift", "Late Entry", "Holiday", "Weekly Off"});
        combo->setCursor(Qt::PointingHandCursor);
        if (attMap.contains(g.id)) {
            combo->setCurrentText(attMap[g.id].first);
        }
        connect(combo, &QComboBox::currentTextChanged, this, [this, combo](const QString& s) {
            combo->setStyleSheet(statusStyleSheet(s));
            updateDailyStats();
        });
        combo->setStyleSheet(statusStyleSheet(combo->currentText()));
        m_dailyTable->setCellWidget(row, 4, combo);
        m_statusCombos[g.id] = combo;

        // Notes
        auto* notesEdit = new QLineEdit;
        notesEdit->setPlaceholderText("Notes...");
        notesEdit->setStyleSheet("QLineEdit { background-color: #151C25; border: 1px solid #252D3A; "
                                 "border-radius: 4px; padding: 4px 8px; color: #E8E4DC; font-size: 12px; } "
                                 "QLineEdit:focus { border: 1px solid #D4B44C; }");
        if (attMap.contains(g.id)) {
            notesEdit->setText(attMap[g.id].second);
        }
        m_dailyTable->setCellWidget(row, 5, notesEdit);
        m_notesEdits[g.id] = notesEdit;
    }

    updateDailyStats();
}

void AttendanceWidget::saveDailyAttendance()
{
    auto& db = DatabaseManager::instance();
    QString dateStr = m_dailyDate->date().toString("yyyy-MM-dd");
    int siteId = m_dailySiteCombo->currentData().toInt();

    int saved = 0, cleared = 0, skipped = 0;

    for (auto it = m_statusCombos.begin(); it != m_statusCombos.end(); ++it) {
        int guardId = it.key();
        QString status = it.value()->currentText();
        QString notes = m_notesEdits.contains(guardId) ? m_notesEdits[guardId]->text().trimmed() : "";

        if (status.isEmpty()) {
            db.executeNonQuery("DELETE FROM Attendance WHERE guard_id = :gid AND date = :date",
                               {{":gid", guardId}, {":date", dateStr}});
            cleared++;
            continue;
        }

        int recordSiteId = siteId;
        if (siteId == 0) {
            auto sq = db.execute("SELECT site_id FROM Guards WHERE id = :id", {{":id", guardId}});
            if (sq.next() && !sq.value("site_id").isNull()) {
                recordSiteId = sq.value("site_id").toInt();
            } else {
                skipped++;
                continue;
            }
        }

        int lateFlag = (status == "Late Entry") ? 1 : 0;

        bool ok = db.executeNonQuery(
            "INSERT INTO Attendance (guard_id, site_id, date, status, late_entry, notes, marked_by) "
            "VALUES (:gid, :sid, :date, :status, :late, :notes, :marked) "
            "ON CONFLICT(guard_id, date) DO UPDATE SET "
            "site_id = excluded.site_id, status = excluded.status, "
            "late_entry = excluded.late_entry, notes = excluded.notes, marked_by = excluded.marked_by",
            {{":gid", guardId}, {":sid", recordSiteId}, {":date", dateStr},
             {":status", status}, {":late", lateFlag}, {":notes", notes},
             {":marked", m_userId > 0 ? m_userId : QVariant()}});

        if (ok) saved++;
    }

    QMessageBox::information(this, "Attendance Saved",
        QString("Attendance for %1 saved.\n\nSaved: %2  |  Cleared: %3  |  Skipped: %4")
            .arg(dateStr).arg(saved).arg(cleared).arg(skipped));
    updateDailyStats();
}

void AttendanceWidget::markAllPresent()
{
    for (auto it = m_statusCombos.begin(); it != m_statusCombos.end(); ++it) {
        it.value()->setCurrentText("Present");
    }
}

void AttendanceWidget::clearAllStatus()
{
    for (auto it = m_statusCombos.begin(); it != m_statusCombos.end(); ++it) {
        it.value()->setCurrentText("");
    }
}

void AttendanceWidget::updateDailyStats()
{
    int total = m_statusCombos.size();
    int present = 0, absent = 0, halfDay = 0, leave = 0;
    int nightShift = 0, lateEntry = 0, holiday = 0, weeklyOff = 0, unmarked = 0;

    for (auto it = m_statusCombos.begin(); it != m_statusCombos.end(); ++it) {
        QString s = it.value()->currentText();
        if (s == "Present")          present++;
        else if (s == "Absent")      absent++;
        else if (s == "Half Day")    halfDay++;
        else if (s == "Leave")       leave++;
        else if (s == "Night Shift") nightShift++;
        else if (s == "Late Entry")  lateEntry++;
        else if (s == "Holiday")     holiday++;
        else if (s == "Weekly Off")  weeklyOff++;
        else unmarked++;
    }

    m_statsLabel->setText(
        QString("Total: %1 | Present: %2 | Absent: %3 | Half Day: %4 | "
                "Leave: %5 | Night: %6 | Late: %7 | Holiday: %8 | Off: %9 | Unmarked: %10")
            .arg(total).arg(present).arg(absent).arg(halfDay)
            .arg(leave).arg(nightShift).arg(lateEntry)
            .arg(holiday).arg(weeklyOff).arg(unmarked));
}

QWidget* AttendanceWidget::buildMonthlyTab()
{
    auto* tab = new QWidget;
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    // Controls
    auto* controlRow = new QHBoxLayout;
    controlRow->setSpacing(12);

    auto* monthLbl = new QLabel("Month:");
    monthLbl->setStyleSheet("color: #8B95A5; font-weight: 600;");
    controlRow->addWidget(monthLbl);

    m_monthCombo = new QComboBox;
    m_monthCombo->addItems({"January", "February", "March", "April", "May", "June",
                            "July", "August", "September", "October", "November", "December"});
    m_monthCombo->setCurrentIndex(QDate::currentDate().month() - 1);
    m_monthCombo->setFixedWidth(130);
    connect(m_monthCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AttendanceWidget::loadMonthlyRegister);
    controlRow->addWidget(m_monthCombo);

    auto* yearLbl = new QLabel("Year:");
    yearLbl->setStyleSheet("color: #8B95A5; font-weight: 600;");
    controlRow->addWidget(yearLbl);

    m_yearSpin = new QSpinBox;
    m_yearSpin->setRange(2020, 2035);
    m_yearSpin->setValue(QDate::currentDate().year());
    m_yearSpin->setFixedWidth(90);
    connect(m_yearSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AttendanceWidget::loadMonthlyRegister);
    controlRow->addWidget(m_yearSpin);
    controlRow->addSpacing(16);

    auto* siteLbl2 = new QLabel("Site:");
    siteLbl2->setStyleSheet("color: #8B95A5; font-weight: 600;");
    controlRow->addWidget(siteLbl2);

    m_monthlySiteCombo = new QComboBox;
    m_monthlySiteCombo->setFixedWidth(220);
    connect(m_monthlySiteCombo, &QComboBox::currentTextChanged, this,
            [this](const QString&) { loadMonthlyRegister(); });
    controlRow->addWidget(m_monthlySiteCombo);
    controlRow->addStretch();

    m_exportBtn = new QPushButton("Export CSV");
    m_exportBtn->setObjectName("SecondaryButton");
    m_exportBtn->setFixedSize(120, 36);
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    connect(m_exportBtn, &QPushButton::clicked, this, &AttendanceWidget::exportMonthlyCSV);
    controlRow->addWidget(m_exportBtn);
    layout->addLayout(controlRow);

    // Legend
    auto* legendRow = new QHBoxLayout;
    legendRow->setSpacing(16);

    auto makeLegend = [&](const QString& letter, const QString& label, const QColor& bg, const QColor& fg) {
        auto* lbl = new QLabel(QString(" %1 %2 ").arg(letter, label));
        lbl->setStyleSheet(QString("background-color: %1; color: %2; border-radius: 3px; "
                                   "padding: 2px 8px; font-size: 11px; font-weight: 700;")
                                   .arg(bg.name(), fg.name()));
        legendRow->addWidget(lbl);
    };

    makeLegend("P", "Present",     QColor("#1A3A1A"), QColor("#4ADE80"));
    makeLegend("A", "Absent",      QColor("#3A1A1A"), QColor("#E85454"));
    makeLegend("H", "Half Day",    QColor("#3A3A1A"), QColor("#FBBF24"));
    makeLegend("L", "Leave",       QColor("#1A1A3A"), QColor("#60A5FA"));
    makeLegend("N", "Night Shift", QColor("#2A1A3A"), QColor("#A78BFA"));
    makeLegend("T", "Late Entry",  QColor("#3A2A1A"), QColor("#FB923C"));
    makeLegend("X", "Holiday",     QColor("#1A2A2A"), QColor("#67E8F9"));
    makeLegend("W", "Weekly Off",  QColor("#1A1A2A"), QColor("#94A3B8"));
    legendRow->addStretch();
    layout->addLayout(legendRow);

    // Table
    m_monthlyTable = new QTableWidget;
    m_monthlyTable->setAlternatingRowColors(true);
    m_monthlyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_monthlyTable->verticalHeader()->setVisible(false);
    m_monthlyTable->setShowGrid(true);
    m_monthlyTable->setGridStyle(Qt::SolidLine);
    layout->addWidget(m_monthlyTable, 1);

    return tab;
}

void AttendanceWidget::loadMonthlyRegister()
{
    auto& db = DatabaseManager::instance();

    // Populate monthly site combo
    if (m_monthlySiteCombo->count() == 0) {
        m_monthlySiteCombo->blockSignals(true);
        m_monthlySiteCombo->addItem("All Sites", 0);
        auto sites = db.execute("SELECT id, site_name FROM Sites WHERE status = 'Active' ORDER BY site_name");
        while (sites.next()) {
            m_monthlySiteCombo->addItem(sites.value("site_name").toString(), sites.value("id").toInt());
        }
        m_monthlySiteCombo->blockSignals(false);
    }

    int month = m_monthCombo->currentIndex() + 1;
    int year = m_yearSpin->value();
    int siteId = m_monthlySiteCombo->currentData().toInt();
    int daysInMonth = QDate(year, month, 1).daysInMonth();

    QString startDate = QDate(year, month, 1).toString("yyyy-MM-dd");
    QString endDate = QDate(year, month, daysInMonth).toString("yyyy-MM-dd");

    // Get guards
    struct GuardInfo { int id; QString code; QString name; QString site; };
    QList<GuardInfo> guardList;

    QString gSql = "SELECT g.id, g.guard_code, g.full_name, COALESCE(s.site_name, '') AS site_name "
                   "FROM Guards g LEFT JOIN Sites s ON g.site_id = s.id "
                   "WHERE g.status = 'Active' AND g.site_id IS NOT NULL ";
    QVariantMap gParams;
    if (siteId > 0) {
        gSql += "AND g.site_id = :siteId ";
        gParams[":siteId"] = siteId;
    }
    gSql += "ORDER BY g.full_name";

    auto gQuery = db.execute(gSql, gParams);
    while (gQuery.next()) {
        guardList.append({gQuery.value("id").toInt(), gQuery.value("guard_code").toString(),
                          gQuery.value("full_name").toString(), gQuery.value("site_name").toString()});
    }

    // Get attendance data for the month
    auto attQuery = db.execute(
        "SELECT guard_id, date, status FROM Attendance WHERE date BETWEEN :start AND :end",
        {{":start", startDate}, {":end", endDate}});

    QMap<int, QMap<int, QString>> attMap;
    while (attQuery.next()) {
        int gid = attQuery.value("guard_id").toInt();
        int day = QDate::fromString(attQuery.value("date").toString(), "yyyy-MM-dd").day();
        attMap[gid][day] = attQuery.value("status").toString();
    }

    // Build table
    int fixedCols = 3;
    int summaryCols = 6;
    int totalCols = fixedCols + daysInMonth + summaryCols;

    m_monthlyTable->clear();
    m_monthlyTable->setColumnCount(totalCols);
    m_monthlyTable->setRowCount(guardList.size());

    QStringList headers;
    headers << "Code" << "Name" << "Site";
    for (int d = 1; d <= daysInMonth; ++d) headers << QString::number(d);
    headers << "P" << "A" << "H" << "L" << "N" << "Total";
    m_monthlyTable->setHorizontalHeaderLabels(headers);

    m_monthlyTable->setColumnWidth(0, 80);
    m_monthlyTable->setColumnWidth(1, 150);
    m_monthlyTable->setColumnWidth(2, 120);
    for (int d = 0; d < daysInMonth; ++d) {
        m_monthlyTable->setColumnWidth(fixedCols + d, 30);
        QDate dayDate(year, month, d + 1);
        auto* hItem = new QTableWidgetItem(QString::number(d + 1));
        hItem->setToolTip(dayDate.toString("dddd, MMMM d"));
        if (dayDate.dayOfWeek() >= 6) hItem->setForeground(QBrush(QColor("#E85454")));
        if (dayDate == QDate::currentDate()) hItem->setForeground(QBrush(QColor("#D4B44C")));
        m_monthlyTable->setHorizontalHeaderItem(fixedCols + d, hItem);
    }

    int sumStart = fixedCols + daysInMonth;
    for (int s = 0; s < summaryCols; ++s) m_monthlyTable->setColumnWidth(sumStart + s, 45);

    // Populate rows
    for (int row = 0; row < guardList.size(); ++row) {
        const auto& g = guardList[row];

        auto setCell = [&](int col, const QString& text, const QColor& bg, const QColor& fg, Qt::Alignment align) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(align);
            if (bg.isValid()) item->setBackground(QBrush(bg));
            if (fg.isValid()) item->setForeground(QBrush(fg));
            m_monthlyTable->setItem(row, col, item);
        };

        setCell(0, g.code, QColor(), QColor(), Qt::AlignLeft | Qt::AlignVCenter);
        setCell(1, g.name, QColor(), QColor(), Qt::AlignLeft | Qt::AlignVCenter);
        setCell(2, g.site, QColor(), QColor(), Qt::AlignLeft | Qt::AlignVCenter);

        int cntP = 0, cntA = 0, cntH = 0, cntL = 0, cntN = 0;

        for (int d = 1; d <= daysInMonth; ++d) {
            QString status = attMap[g.id].value(d, "");
            QString letter = statusLetter(status);
            QColor bg = statusBgColor(status);
            QColor fg = statusFgColor(status);
            setCell(fixedCols + d - 1, letter, bg, fg, Qt::AlignCenter);

            if (status == "Present")       cntP++;
            else if (status == "Absent")   cntA++;
            else if (status == "Half Day") cntH++;
            else if (status == "Leave")    cntL++;
            else if (status == "Night Shift") cntN++;
        }

        setCell(sumStart,     QString::number(cntP), QColor("#1A3A1A"), QColor("#4ADE80"), Qt::AlignCenter);
        setCell(sumStart + 1, QString::number(cntA), QColor("#3A1A1A"), QColor("#E85454"), Qt::AlignCenter);
        setCell(sumStart + 2, QString::number(cntH), QColor("#3A3A1A"), QColor("#FBBF24"), Qt::AlignCenter);
        setCell(sumStart + 3, QString::number(cntL), QColor("#1A1A3A"), QColor("#60A5FA"), Qt::AlignCenter);
        setCell(sumStart + 4, QString::number(cntN), QColor("#2A1A3A"), QColor("#A78BFA"), Qt::AlignCenter);
        setCell(sumStart + 5, QString::number(cntP + cntN + cntH), QColor("#1A2A1A"), QColor("#D4B44C"), Qt::AlignCenter);
    }
}

void AttendanceWidget::exportMonthlyCSV()
{
    if (m_monthlyTable->rowCount() == 0) {
        QMessageBox::information(this, "No Data", "No attendance data to export.");
        return;
    }

    QString defaultName = QString("attendance_%1_%2.csv")
        .arg(m_monthCombo->currentText().toLower()).arg(m_yearSpin->value());

    QDir().mkpath(QCoreApplication::applicationDirPath() + "/reports");

    QString filePath = QFileDialog::getSaveFileName(
        this, "Export Monthly Attendance",
        QCoreApplication::applicationDirPath() + "/reports/" + defaultName,
        "CSV Files (*.csv);;All Files (*)");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file for writing.");
        return;
    }

    QTextStream out(&file);

    // Header
    QStringList headerParts;
    for (int col = 0; col < m_monthlyTable->columnCount(); ++col) {
        auto* hItem = m_monthlyTable->horizontalHeaderItem(col);
        QString text = hItem ? hItem->text() : "";
        if (text.contains(',')) text = "\"" + text + "\"";
        headerParts << text;
    }
    out << headerParts.join(",") << "\n";

    // Data
    for (int row = 0; row < m_monthlyTable->rowCount(); ++row) {
        QStringList rowParts;
        for (int col = 0; col < m_monthlyTable->columnCount(); ++col) {
            auto* item = m_monthlyTable->item(row, col);
            QString text = item ? item->text() : "";
            if (text.contains(',')) text = "\"" + text + "\"";
            rowParts << text;
        }
        out << rowParts.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, "Export Successful",
        QString("Monthly attendance exported to:\n\n%1").arg(filePath));
}

void AttendanceWidget::refresh()
{
    loadDailyAttendance();
}

// ═══════════════════════════════════════════════════════════════
//  STATUS HELPERS
// ═══════════════════════════════════════════════════════════════

QString AttendanceWidget::statusLetter(const QString& status) const
{
    if (status == "Present")       return "P";
    if (status == "Absent")       return "A";
    if (status == "Half Day")     return "H";
    if (status == "Leave")        return "L";
    if (status == "Night Shift")  return "N";
    if (status == "Late Entry")   return "T";
    if (status == "Holiday")      return "X";
    if (status == "Weekly Off")   return "W";
    return "";
}

QColor AttendanceWidget::statusBgColor(const QString& status) const
{
    if (status == "Present")       return QColor("#1A3A1A");
    if (status == "Absent")       return QColor("#3A1A1A");
    if (status == "Half Day")     return QColor("#3A3A1A");
    if (status == "Leave")        return QColor("#1A1A3A");
    if (status == "Night Shift")  return QColor("#2A1A3A");
    if (status == "Late Entry")   return QColor("#3A2A1A");
    if (status == "Holiday")      return QColor("#1A2A2A");
    if (status == "Weekly Off")   return QColor("#1A1A2A");
    return QColor();
}

QColor AttendanceWidget::statusFgColor(const QString& status) const
{
    if (status == "Present")       return QColor("#4ADE80");
    if (status == "Absent")       return QColor("#E85454");
    if (status == "Half Day")     return QColor("#FBBF24");
    if (status == "Leave")        return QColor("#60A5FA");
    if (status == "Night Shift")  return QColor("#A78BFA");
    if (status == "Late Entry")   return QColor("#FB923C");
    if (status == "Holiday")      return QColor("#67E8F9");
    if (status == "Weekly Off")   return QColor("#94A3B8");
    return QColor("#6B7585");
}

QString AttendanceWidget::statusStyleSheet(const QString& status) const
{
    QString bg = "#151C25", fg = "#6B7585", border = "#252D3A";

    if (status == "Present")        { bg = "#1A3A1A"; fg = "#4ADE80"; border = "#2A4A2A"; }
    else if (status == "Absent")    { bg = "#3A1A1A"; fg = "#E85454"; border = "#4A2A2A"; }
    else if (status == "Half Day")  { bg = "#3A3A1A"; fg = "#FBBF24"; border = "#4A4A2A"; }
    else if (status == "Leave")     { bg = "#1A1A3A"; fg = "#60A5FA"; border = "#2A2A4A"; }
    else if (status == "Night Shift") { bg = "#2A1A3A"; fg = "#A78BFA"; border = "#3A2A4A"; }
    else if (status == "Late Entry")  { bg = "#3A2A1A"; fg = "#FB923C"; border = "#4A3A2A"; }
    else if (status == "Holiday")     { bg = "#1A2A2A"; fg = "#67E8F9"; border = "#2A3A3A"; }
    else if (status == "Weekly Off")  { bg = "#1A1A2A"; fg = "#94A3B8"; border = "#2A2A3A"; }

    return QString(
        "QComboBox { background-color: %1; color: %2; border: 1px solid %3; "
        "border-radius: 4px; padding: 4px 8px; font-weight: 600; font-size: 12px; } "
        "QComboBox::drop-down { border: none; width: 20px; } "
        "QComboBox QAbstractItemView { background-color: #151C25; border: 1px solid #252D3A; "
        "selection-background-color: #1A2740; selection-color: #D4B44C; }"
    ).arg(bg, fg, border);
}
