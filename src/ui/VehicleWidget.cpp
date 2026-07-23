#include "VehicleWidget.h"
#include "VehicleDialog.h"
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

VehicleWidget::VehicleWidget(QWidget* parent) : QWidget(parent)
{
    buildUI();
    loadVehicles();
}

void VehicleWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Vehicle Register");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    auto* subtitle = new QLabel("Track vehicle entries and exits at all sites");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    // Header row
    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(8);

    m_addBtn = new QPushButton("+ New Vehicle");
    m_addBtn->setObjectName("PrimaryButton");
    m_addBtn->setFixedSize(140, 36);
    m_addBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addBtn, &QPushButton::clicked, this, &VehicleWidget::addVehicle);
    headerRow->addWidget(m_addBtn);

    m_editBtn = new QPushButton("Edit");
    m_editBtn->setObjectName("SecondaryButton");
    m_editBtn->setFixedSize(70, 36);
    m_editBtn->setCursor(Qt::PointingHandCursor);
    connect(m_editBtn, &QPushButton::clicked, this, &VehicleWidget::editVehicle);
    headerRow->addWidget(m_editBtn);

    m_exitBtn = new QPushButton("Mark Exit");
    m_exitBtn->setObjectName("SecondaryButton");
    m_exitBtn->setFixedSize(100, 36);
    m_exitBtn->setCursor(Qt::PointingHandCursor);
    m_exitBtn->setStyleSheet(
        "QPushButton { background-color: #1A3A1A; color: #4ADE80; border: 1px solid #2A4A2A; "
        "border-radius: 6px; padding: 6px 16px; font-weight: 600; }"
        "QPushButton:hover { background-color: #2A4A2A; }");
    connect(m_exitBtn, &QPushButton::clicked, this, &VehicleWidget::markExit);
    headerRow->addWidget(m_exitBtn);

    m_deleteBtn = new QPushButton("Delete");
    m_deleteBtn->setObjectName("DangerButton");
    m_deleteBtn->setFixedSize(80, 36);
    m_deleteBtn->setCursor(Qt::PointingHandCursor);
    connect(m_deleteBtn, &QPushButton::clicked, this, &VehicleWidget::deleteVehicle);
    headerRow->addWidget(m_deleteBtn);

    m_exportBtn = new QPushButton("Export CSV");
    m_exportBtn->setObjectName("SecondaryButton");
    m_exportBtn->setFixedSize(110, 36);
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    connect(m_exportBtn, &QPushButton::clicked, this, &VehicleWidget::exportCSV);
    headerRow->addWidget(m_exportBtn);

    headerRow->addStretch();

    m_countLabel = new QLabel;
    m_countLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    headerRow->addWidget(m_countLabel);

    mainLayout->addLayout(headerRow);

    // Filter row
    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by vehicle no, driver, purpose...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &VehicleWidget::filterVehicles);
    filterRow->addWidget(m_searchEdit, 1);

    m_siteFilter = new QComboBox;
    m_siteFilter->addItem("All Sites", 0);
    auto& db = DatabaseManager::instance();
    auto sites = db.execute("SELECT id, site_name FROM Sites WHERE status = 'Active' ORDER BY site_name");
    while (sites.next()) {
        m_siteFilter->addItem(sites.value("site_name").toString(), sites.value("id").toInt());
    }
    m_siteFilter->setFixedWidth(160);
    connect(m_siteFilter, &QComboBox::currentTextChanged, this,
            [this](const QString&) { filterVehicles(m_searchEdit->text()); });
    filterRow->addWidget(m_siteFilter);

    m_statusFilter = new QComboBox;
    m_statusFilter->addItems({"All", "Inside", "Exited"});
    m_statusFilter->setFixedWidth(100);
    connect(m_statusFilter, &QComboBox::currentTextChanged, this,
            [this](const QString&) { filterVehicles(m_searchEdit->text()); });
    filterRow->addWidget(m_statusFilter);

    mainLayout->addLayout(filterRow);

    // Summary
    m_summaryLabel = new QLabel;
    m_summaryLabel->setStyleSheet("color: #D4B44C; font-size: 13px; font-weight: 600;");
    mainLayout->addWidget(m_summaryLabel);

    // Table
    m_table = new QTableWidget;
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(true);

    QStringList cols = {"ID", "Site", "Vehicle No", "Type", "Driver", "Mobile",
                        "Purpose", "Entry Time", "Exit Time", "Status", "Notes"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);

    m_table->setColumnWidth(1, 110);
    m_table->setColumnWidth(2, 120);
    m_table->setColumnWidth(3, 80);
    m_table->setColumnWidth(4, 130);
    m_table->setColumnWidth(5, 110);
    m_table->setColumnWidth(6, 130);
    m_table->setColumnWidth(7, 140);
    m_table->setColumnWidth(8, 140);
    m_table->setColumnWidth(9, 70);
    m_table->horizontalHeader()->setStretchLastSection(true);

    connect(m_table, &QTableWidget::cellDoubleClicked, this, &VehicleWidget::editVehicle);

    mainLayout->addWidget(m_table, 1);
}

void VehicleWidget::loadVehicles()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT v.*, s.site_name FROM Vehicles v "
        "JOIN Sites s ON v.site_id = s.id "
        "ORDER BY v.created_at DESC"
    );

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM Vehicles");
    if (cc.next()) count = cc.value(0).toInt();
    m_table->setRowCount(count);
    m_table->setSortingEnabled(false);

    int row = 0;
    int inside = 0, exited = 0;

    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        setItem(1, query.value("site_name").toString());
        setItem(2, query.value("vehicle_no").toString());
        setItem(3, query.value("vehicle_type").toString());
        setItem(4, query.value("driver_name").toString());
        setItem(5, query.value("driver_mobile").toString());
        setItem(6, query.value("purpose").toString());
        setItem(7, query.value("entry_time").toString());

        QString exitTime = query.value("exit_time").toString();
        setItem(8, exitTime.isEmpty() ? "-" : exitTime);

        // Status
        bool hasExited = !exitTime.isEmpty();
        auto* statusItem = new QTableWidgetItem(hasExited ? "Exited" : "Inside");
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (hasExited) { statusItem->setForeground(QColor("#6B7585")); exited++; }
        else           { statusItem->setForeground(QColor("#4ADE80")); inside++; }
        m_table->setItem(row, 9, statusItem);

        setItem(10, query.value("notes").toString());

        row++;
    }

    m_table->setSortingEnabled(true);
    m_countLabel->setText(QString("%1 entries").arg(row));
    m_summaryLabel->setText(
        QString("Total: %1 | Currently Inside: %2 | Exited: %3")
            .arg(row).arg(inside).arg(exited)
    );
}

void VehicleWidget::refresh()
{
    loadVehicles();
}

void VehicleWidget::addVehicle()
{
    VehicleDialog dlg(this, -1);
    if (dlg.exec() == QDialog::Accepted) loadVehicles();
}

void VehicleWidget::editVehicle()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Select a vehicle entry to edit.");
        return;
    }
    int id = m_table->item(items.first()->row(), 0)->text().toInt();
    VehicleDialog dlg(this, id);
    if (dlg.exec() == QDialog::Accepted) loadVehicles();
}

void VehicleWidget::markExit()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Select a vehicle to mark exit.");
        return;
    }
    int row = items.first()->row();
    int id = m_table->item(row, 0)->text().toInt();
    QString vno = m_table->item(row, 2)->text();
    QString status = m_table->item(row, 9)->text();

    if (status == "Exited") {
        QMessageBox::information(this, "Already Exited",
            QString("Vehicle %1 has already exited.").arg(vno));
        return;
    }

    auto result = QMessageBox::question(this, "Mark Exit",
        QString("Mark exit for vehicle %1?\nTime: %2").arg(vno, QTime::currentTime().toString("HH:mm")),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        QString exitTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
        db.executeNonQuery(
            "UPDATE Vehicles SET exit_time = :time WHERE id = :id",
            {{":time", exitTime}, {":id", id}}
        );
        loadVehicles();
    }
}

void VehicleWidget::deleteVehicle()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Select a vehicle entry to delete.");
        return;
    }
    int row = items.first()->row();
    int id = m_table->item(row, 0)->text().toInt();
    QString vno = m_table->item(row, 2)->text();

    auto result = QMessageBox::question(this, "Delete",
        QString("Delete vehicle entry for %1?").arg(vno),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("DELETE FROM Vehicles WHERE id = :id", {{":id", id}});
        loadVehicles();
    }
}

void VehicleWidget::filterVehicles(const QString& text)
{
    QString searchText = text.toLower();
    int siteId = m_siteFilter->currentData().toInt();
    QString statusFilter = m_statusFilter->currentText();

    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool textMatch = searchText.isEmpty();
        bool siteMatch = (siteId == 0);
        bool statusMatch = (statusFilter == "All");

        if (!textMatch) {
            for (int col = 1; col <= 8; ++col) {
                auto* item = m_table->item(row, col);
                if (item && item->text().toLower().contains(searchText)) {
                    textMatch = true; break;
                }
            }
        }

        if (!siteMatch) {
            auto* item = m_table->item(row, 1);
            if (item) siteMatch = (item->text() == m_siteFilter->currentText());
        }

        if (!statusMatch) {
            auto* item = m_table->item(row, 9);
            if (item) statusMatch = (item->text() == statusFilter);
        }

        m_table->setRowHidden(row, !(textMatch && siteMatch && statusMatch));
    }
}

void VehicleWidget::exportCSV()
{
    if (m_table->rowCount() == 0) {
        QMessageBox::information(this, "No Data", "No vehicle data to export.");
        return;
    }

    QDir().mkpath(QCoreApplication::applicationDirPath() + "/reports");
    QString filePath = QFileDialog::getSaveFileName(
        this, "Export Vehicle Report",
        QCoreApplication::applicationDirPath() + "/reports/vehicles_" +
            QDate::currentDate().toString("yyyy-MM-dd") + ".csv",
        "CSV Files (*.csv);;All Files (*)"
    );
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file.");
        return;
    }

    QTextStream out(&file);

    QStringList headers;
    QList<int> visibleCols;
    for (int col = 0; col < m_table->columnCount(); ++col) {
        if (!m_table->isColumnHidden(col)) {
            visibleCols << col;
            auto* h = m_table->horizontalHeaderItem(col);
            headers << (h ? h->text() : "");
        }
    }
    out << headers.join(",") << "\n";

    for (int row = 0; row < m_table->rowCount(); ++row) {
        if (m_table->isRowHidden(row)) continue;
        QStringList rowParts;
        for (int col : visibleCols) {
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
        QString("Vehicle report exported to:\n\n%1").arg(filePath));
}
