#include "EquipmentWidget.h"
#include "EquipmentDialog.h"
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

EquipmentWidget::EquipmentWidget(QWidget* parent) : QWidget(parent)
{
    buildUI();
    loadInventory();
    loadIssuance();
}

void EquipmentWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Equipment Management");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    auto* subtitle = new QLabel("Track equipment inventory, issue to guards, manage returns and repairs");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    auto* tabWidget = new QTabWidget;
    tabWidget->addTab(buildInventoryTab(), "Inventory");
    tabWidget->addTab(buildIssuanceTab(), "Issuance Log");

    connect(tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 0) loadInventory();
        if (index == 1) loadIssuance();
    });

    mainLayout->addWidget(tabWidget, 1);
}

QWidget* EquipmentWidget::buildInventoryTab()
{
    auto* tab = new QWidget;
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(8);

    auto* addBtn = new QPushButton("+ Add Equipment");
    addBtn->setObjectName("PrimaryButton");
    addBtn->setFixedSize(150, 36);
    addBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &EquipmentWidget::addItem);
    headerRow->addWidget(addBtn);

    auto* editBtn = new QPushButton("Edit");
    editBtn->setObjectName("SecondaryButton");
    editBtn->setFixedSize(70, 36);
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, &EquipmentWidget::editItem);
    headerRow->addWidget(editBtn);

    auto* delBtn = new QPushButton("Delete");
    delBtn->setObjectName("DangerButton");
    delBtn->setFixedSize(80, 36);
    delBtn->setCursor(Qt::PointingHandCursor);
    connect(delBtn, &QPushButton::clicked, this, &EquipmentWidget::deleteItem);
    headerRow->addWidget(delBtn);

    headerRow->addStretch();

    m_invSummary = new QLabel;
    m_invSummary->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    headerRow->addWidget(m_invSummary);

    layout->addLayout(headerRow);

    m_invTable = new QTableWidget;
    m_invTable->setAlternatingRowColors(true);
    m_invTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_invTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_invTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_invTable->verticalHeader()->setVisible(false);
    m_invTable->setShowGrid(false);
    m_invTable->setSortingEnabled(true);

    QStringList cols = {"ID", "Code", "Type", "Description", "Serial No.",
                        "Purchase Date", "Condition", "Status", "Notes", "Created"};
    m_invTable->setColumnCount(cols.size());
    m_invTable->setHorizontalHeaderLabels(cols);
    m_invTable->setColumnHidden(0, true);
    m_invTable->setColumnWidth(1, 100);
    m_invTable->setColumnWidth(2, 140);
    m_invTable->setColumnWidth(3, 200);
    m_invTable->setColumnWidth(4, 120);
    m_invTable->setColumnWidth(5, 100);
    m_invTable->setColumnWidth(6, 90);
    m_invTable->setColumnWidth(7, 90);
    m_invTable->setColumnWidth(8, 200);
    m_invTable->horizontalHeader()->setStretchLastSection(true);

    connect(m_invTable, &QTableWidget::cellDoubleClicked, this, &EquipmentWidget::editItem);

    layout->addWidget(m_invTable, 1);
    return tab;
}

QWidget* EquipmentWidget::buildIssuanceTab()
{
    auto* tab = new QWidget;
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    // Action row
    auto* actionRow = new QHBoxLayout;
    actionRow->setSpacing(8);

    m_issueBtn = new QPushButton("Issue Equipment");
    m_issueBtn->setObjectName("PrimaryButton");
    m_issueBtn->setFixedSize(150, 36);
    m_issueBtn->setCursor(Qt::PointingHandCursor);
    connect(m_issueBtn, &QPushButton::clicked, this, &EquipmentWidget::issueEquipment);
    actionRow->addWidget(m_issueBtn);

    m_returnBtn = new QPushButton("Return");
    m_returnBtn->setObjectName("SecondaryButton");
    m_returnBtn->setFixedSize(90, 36);
    m_returnBtn->setCursor(Qt::PointingHandCursor);
    m_returnBtn->setStyleSheet(
        "QPushButton { background-color: #1A3A1A; color: #4ADE80; border: 1px solid #2A4A2A; "
        "border-radius: 6px; padding: 6px 16px; font-weight: 600; }"
        "QPushButton:hover { background-color: #2A4A2A; }");
    connect(m_returnBtn, &QPushButton::clicked, this, &EquipmentWidget::returnEquipment);
    actionRow->addWidget(m_returnBtn);

    m_repairBtn = new QPushButton("Mark Under Repair");
    m_repairBtn->setObjectName("SecondaryButton");
    m_repairBtn->setFixedSize(150, 36);
    m_repairBtn->setCursor(Qt::PointingHandCursor);
    m_repairBtn->setStyleSheet(
        "QPushButton { background-color: #3A2A1A; color: #FB923C; border: 1px solid #5A3A1A; "
        "border-radius: 6px; padding: 6px 16px; font-weight: 600; }"
        "QPushButton:hover { background-color: #4A3A2A; }");
    connect(m_repairBtn, &QPushButton::clicked, this, &EquipmentWidget::markRepair);
    actionRow->addWidget(m_repairBtn);

    m_exportBtn = new QPushButton("Export CSV");
    m_exportBtn->setObjectName("SecondaryButton");
    m_exportBtn->setFixedSize(110, 36);
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    connect(m_exportBtn, &QPushButton::clicked, this, &EquipmentWidget::exportCSV);
    actionRow->addWidget(m_exportBtn);

    actionRow->addStretch();
    layout->addLayout(actionRow);

    // Filter row
    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by guard name, equipment code...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &EquipmentWidget::filterIssuance);
    filterRow->addWidget(m_searchEdit, 1);

    m_statusFilter = new QComboBox;
    m_statusFilter->addItems({"All", "Issued", "Returned"});
    m_statusFilter->setFixedWidth(110);
    connect(m_statusFilter, &QComboBox::currentTextChanged, this,
            [this](const QString&) { filterIssuance(m_searchEdit->text()); });
    filterRow->addWidget(m_statusFilter);

    layout->addLayout(filterRow);

    m_issSummary = new QLabel;
    m_issSummary->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    layout->addWidget(m_issSummary);

    // Table
    m_issTable = new QTableWidget;
    m_issTable->setAlternatingRowColors(true);
    m_issTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_issTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_issTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_issTable->verticalHeader()->setVisible(false);
    m_issTable->setShowGrid(false);
    m_issTable->setSortingEnabled(true);

    QStringList cols = {"ID", "Equipment ID", "Code", "Guard Name", "Equipment", "Type",
                        "Issue Date", "Condition Out", "Status", "Return Date",
                        "Condition In", "Notes"};
    m_issTable->setColumnCount(cols.size());
    m_issTable->setHorizontalHeaderLabels(cols);
    m_issTable->setColumnHidden(0, true);
    m_issTable->setColumnHidden(1, true);

    m_issTable->setColumnWidth(2, 90);
    m_issTable->setColumnWidth(3, 160);
    m_issTable->setColumnWidth(4, 120);
    m_issTable->setColumnWidth(5, 120);
    m_issTable->setColumnWidth(6, 100);
    m_issTable->setColumnWidth(7, 110);
    m_issTable->setColumnWidth(8, 80);
    m_issTable->setColumnWidth(9, 100);
    m_issTable->setColumnWidth(10, 110);
    m_issTable->horizontalHeader()->setStretchLastSection(true);

    layout->addWidget(m_issTable, 1);
    return tab;
}

void EquipmentWidget::loadInventory()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute("SELECT * FROM Equipment ORDER BY equipment_code");

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM Equipment");
    if (cc.next()) count = cc.value(0).toInt();
    m_invTable->setRowCount(count);
    m_invTable->setSortingEnabled(false);

    int row = 0;
    int avail = 0, issued = 0, repair = 0, retired = 0;

    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_invTable->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        setItem(1, query.value("equipment_code").toString());
        setItem(2, query.value("equipment_type").toString());
        setItem(3, query.value("description").toString());
        setItem(4, query.value("serial_number").toString());
        setItem(5, query.value("purchase_date").toString());

        // Condition with color
        QString cond = query.value("condition").toString();
        auto* condItem = new QTableWidgetItem(cond);
        condItem->setTextAlignment(Qt::AlignCenter);
        if (cond == "New")         condItem->setForeground(QColor("#4ADE80"));
        else if (cond == "Good")   condItem->setForeground(QColor("#60A5FA"));
        else if (cond == "Fair")   condItem->setForeground(QColor("#FBBF24"));
        else if (cond == "Poor")   condItem->setForeground(QColor("#FB923C"));
        else if (cond == "Damaged") condItem->setForeground(QColor("#E85454"));
        m_invTable->setItem(row, 6, condItem);

        // Status with color
        QString status = query.value("status").toString();
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "Available")     { statusItem->setForeground(QColor("#4ADE80")); avail++; }
        else if (status == "Issued")   { statusItem->setForeground(QColor("#FBBF24")); issued++; }
        else if (status == "Under Repair") { statusItem->setForeground(QColor("#FB923C")); repair++; }
        else if (status == "Retired")  { statusItem->setForeground(QColor("#6B7585")); retired++; }
        else if (status == "Lost")     { statusItem->setForeground(QColor("#E85454")); }
        m_invTable->setItem(row, 7, statusItem);

        setItem(8, query.value("notes").toString());
        setItem(9, query.value("created_at").toString());

        row++;
    }

    m_invTable->setSortingEnabled(true);
    m_invSummary->setText(
        QString("Total: %1 | Available: %2 | Issued: %3 | Repair: %4 | Retired: %5")
            .arg(row).arg(avail).arg(issued).arg(repair).arg(retired)
    );
}

void EquipmentWidget::loadIssuance()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT ei.*, g.guard_code, g.full_name, e.equipment_code, e.equipment_type "
        "FROM EquipmentIssue ei "
        "JOIN Guards g ON ei.guard_id = g.id "
        "JOIN Equipment e ON ei.equipment_id = e.id "
        "ORDER BY ei.created_at DESC"
    );

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM EquipmentIssue");
    if (cc.next()) count = cc.value(0).toInt();
    m_issTable->setRowCount(count);
    m_issTable->setSortingEnabled(false);

    int row = 0;
    int issuedCount = 0, returnedCount = 0;

    while (query.next()) {
        QString returnDate = query.value("return_date").toString();
        bool isReturned = !returnDate.isEmpty();

        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_issTable->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        setItem(1, query.value("equipment_id").toString());
        setItem(2, query.value("equipment_code").toString());
        setItem(3, query.value("full_name").toString());
        setItem(4, query.value("equipment_code").toString());
        setItem(5, query.value("equipment_type").toString());
        setItem(6, query.value("issue_date").toString());
        setItem(7, query.value("condition_out").toString());

        auto* statusItem = new QTableWidgetItem(isReturned ? "Returned" : "Issued");
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (isReturned) { statusItem->setForeground(QColor("#4ADE80")); returnedCount++; }
        else            { statusItem->setForeground(QColor("#FBBF24")); issuedCount++; }
        m_issTable->setItem(row, 8, statusItem);

        setItem(9, isReturned ? returnDate : "-");
        setItem(10, isReturned ? query.value("condition_in").toString() : "-");
        setItem(11, query.value("notes").toString());

        row++;
    }

    m_issTable->setSortingEnabled(true);
    m_issSummary->setText(
        QString("Total: %1 | Currently Issued: %2 | Returned: %3")
            .arg(row).arg(issuedCount).arg(returnedCount)
    );
}

void EquipmentWidget::refresh()
{
    loadInventory();
    loadIssuance();
}

void EquipmentWidget::addItem()
{
    EquipmentDialog dlg(EquipmentDialog::ItemMode, this, -1);
    if (dlg.exec() == QDialog::Accepted) loadInventory();
}

void EquipmentWidget::editItem()
{
    auto items = m_invTable->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Select equipment to edit.");
        return;
    }
    int id = m_invTable->item(items.first()->row(), 0)->text().toInt();
    EquipmentDialog dlg(EquipmentDialog::ItemMode, this, id);
    if (dlg.exec() == QDialog::Accepted) loadInventory();
}

void EquipmentWidget::deleteItem()
{
    auto items = m_invTable->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Select equipment to delete.");
        return;
    }
    int row = items.first()->row();
    int id = m_invTable->item(row, 0)->text().toInt();
    QString code = m_invTable->item(row, 1)->text();

    auto result = QMessageBox::question(this, "Delete",
        QString("Delete \"%1\" from inventory?").arg(code),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("DELETE FROM Equipment WHERE id = :id", {{":id", id}});
        loadInventory();
    }
}

void EquipmentWidget::issueEquipment()
{
    EquipmentDialog dlg(EquipmentDialog::IssueMode, this);
    if (dlg.exec() == QDialog::Accepted) {
        loadInventory();
        loadIssuance();
    }
}

void EquipmentWidget::returnEquipment()
{
    auto items = m_issTable->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Select an issuance record to return.");
        return;
    }
    int row = items.first()->row();
    int issueId = m_issTable->item(row, 0)->text().toInt();
    QString status = m_issTable->item(row, 8)->text();

    if (status == "Returned") {
        QMessageBox::information(this, "Already Returned", "This equipment has already been returned.");
        return;
    }

    EquipmentDialog dlg(EquipmentDialog::ReturnMode, this, issueId);
    if (dlg.exec() == QDialog::Accepted) {
        loadInventory();
        loadIssuance();
    }
}

void EquipmentWidget::markRepair()
{
    auto items = m_invTable->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Select equipment to mark under repair.");
        return;
    }
    int row = items.first()->row();
    int id = m_invTable->item(row, 0)->text().toInt();
    QString code = m_invTable->item(row, 1)->text();
    QString status = m_invTable->item(row, 7)->text();

    if (status == "Issued") {
        QMessageBox::warning(this, "Cannot Repair", "Return the equipment first before marking for repair.");
        return;
    }

    auto result = QMessageBox::question(this, "Mark Under Repair",
        QString("Mark \"%1\" as under repair?").arg(code),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery(
            "UPDATE Equipment SET status = 'Under Repair' WHERE id = :id", {{":id", id}}
        );
        loadInventory();
    }
}

void EquipmentWidget::filterIssuance(const QString& text)
{
    QString searchText = text.toLower();
    QString statusFilter = m_statusFilter->currentText();

    for (int row = 0; row < m_issTable->rowCount(); ++row) {
        bool textMatch = searchText.isEmpty();
        bool statusMatch = (statusFilter == "All");

        if (!textMatch) {
            for (int col = 2; col <= 6; ++col) {
                auto* item = m_issTable->item(row, col);
                if (item && item->text().toLower().contains(searchText)) {
                    textMatch = true; break;
                }
            }
        }

        if (!statusMatch) {
            auto* item = m_issTable->item(row, 8);
            if (item) statusMatch = (item->text() == statusFilter);
        }

        m_issTable->setRowHidden(row, !(textMatch && statusMatch));
    }
}

void EquipmentWidget::exportCSV()
{
    if (m_issTable->rowCount() == 0) {
        QMessageBox::information(this, "No Data", "No issuance data to export.");
        return;
    }

    QDir().mkpath(QCoreApplication::applicationDirPath() + "/reports");
    QString filePath = QFileDialog::getSaveFileName(
        this, "Export Equipment Report",
        QCoreApplication::applicationDirPath() + "/reports/equipment_report.csv",
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
    for (int col = 0; col < m_issTable->columnCount(); ++col) {
        if (!m_issTable->isColumnHidden(col)) {
            visibleCols << col;
            auto* h = m_issTable->horizontalHeaderItem(col);
            headers << (h ? h->text() : "");
        }
    }
    out << headers.join(",") << "\n";

    for (int row = 0; row < m_issTable->rowCount(); ++row) {
        if (m_issTable->isRowHidden(row)) continue;
        QStringList rowParts;
        for (int col : visibleCols) {
            auto* item = m_issTable->item(row, col);
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
        QString("Equipment report exported to:\n\n%1").arg(filePath));
}
