#include "UniformWidget.h"
#include "UniformDialog.h"
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

UniformWidget::UniformWidget(QWidget* parent) : QWidget(parent)
{
    buildUI();
    loadInventory();
    loadIssuance();
}

void UniformWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Uniform Management");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    auto* subtitle = new QLabel("Track uniform stock, issue to guards, manage returns");
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

QWidget* UniformWidget::buildInventoryTab()
{
    auto* tab = new QWidget;
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(8);

    auto* addBtn = new QPushButton("+ Add Item");
    addBtn->setObjectName("PrimaryButton");
    addBtn->setFixedSize(110, 36);
    addBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &UniformWidget::addItem);
    headerRow->addWidget(addBtn);

    auto* editBtn = new QPushButton("Edit");
    editBtn->setObjectName("SecondaryButton");
    editBtn->setFixedSize(70, 36);
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, &UniformWidget::editItem);
    headerRow->addWidget(editBtn);

    auto* delBtn = new QPushButton("Delete");
    delBtn->setObjectName("DangerButton");
    delBtn->setFixedSize(80, 36);
    delBtn->setCursor(Qt::PointingHandCursor);
    connect(delBtn, &QPushButton::clicked, this, &UniformWidget::deleteItem);
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

    QStringList cols = {"ID", "Item Type", "Size", "Quantity", "Status", "Notes", "Created"};
    m_invTable->setColumnCount(cols.size());
    m_invTable->setHorizontalHeaderLabels(cols);
    m_invTable->setColumnHidden(0, true);
    m_invTable->setColumnWidth(1, 180);
    m_invTable->setColumnWidth(2, 80);
    m_invTable->setColumnWidth(3, 90);
    m_invTable->setColumnWidth(4, 100);
    m_invTable->setColumnWidth(5, 250);
    m_invTable->horizontalHeader()->setStretchLastSection(true);

    connect(m_invTable, &QTableWidget::cellDoubleClicked, this, &UniformWidget::editItem);

    layout->addWidget(m_invTable, 1);
    return tab;
}

QWidget* UniformWidget::buildIssuanceTab()
{
    auto* tab = new QWidget;
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    // Action row
    auto* actionRow = new QHBoxLayout;
    actionRow->setSpacing(8);

    m_issueBtn = new QPushButton("Issue Uniform");
    m_issueBtn->setObjectName("PrimaryButton");
    m_issueBtn->setFixedSize(140, 36);
    m_issueBtn->setCursor(Qt::PointingHandCursor);
    connect(m_issueBtn, &QPushButton::clicked, this, &UniformWidget::issueUniform);
    actionRow->addWidget(m_issueBtn);

    m_returnBtn = new QPushButton("Return");
    m_returnBtn->setObjectName("SecondaryButton");
    m_returnBtn->setFixedSize(90, 36);
    m_returnBtn->setCursor(Qt::PointingHandCursor);
    m_returnBtn->setStyleSheet(
        "QPushButton { background-color: #1A3A1A; color: #4ADE80; border: 1px solid #2A4A2A; "
        "border-radius: 6px; padding: 6px 16px; font-weight: 600; }"
        "QPushButton:hover { background-color: #2A4A2A; }");
    connect(m_returnBtn, &QPushButton::clicked, this, &UniformWidget::returnUniform);
    actionRow->addWidget(m_returnBtn);

    m_exportBtn = new QPushButton("Export CSV");
    m_exportBtn->setObjectName("SecondaryButton");
    m_exportBtn->setFixedSize(110, 36);
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    connect(m_exportBtn, &QPushButton::clicked, this, &UniformWidget::exportCSV);
    actionRow->addWidget(m_exportBtn);

    actionRow->addStretch();
    layout->addLayout(actionRow);

    // Filter row
    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by guard name, item...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &UniformWidget::filterIssuance);
    filterRow->addWidget(m_searchEdit, 1);

    m_statusFilter = new QComboBox;
    m_statusFilter->addItems({"All", "Issued", "Returned"});
    m_statusFilter->setFixedWidth(110);
    connect(m_statusFilter, &QComboBox::currentTextChanged, this,
            [this](const QString&) { filterIssuance(m_searchEdit->text()); });
    filterRow->addWidget(m_statusFilter);

    layout->addLayout(filterRow);

    // Summary
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

    QStringList cols = {"ID", "Uniform ID", "Code", "Guard Name", "Item", "Size",
                        "Issue Date", "Condition Out", "Status", "Return Date",
                        "Condition In", "Notes"};
    m_issTable->setColumnCount(cols.size());
    m_issTable->setHorizontalHeaderLabels(cols);
    m_issTable->setColumnHidden(0, true);
    m_issTable->setColumnHidden(1, true);

    m_issTable->setColumnWidth(2, 80);
    m_issTable->setColumnWidth(3, 160);
    m_issTable->setColumnWidth(4, 140);
    m_issTable->setColumnWidth(5, 60);
    m_issTable->setColumnWidth(6, 100);
    m_issTable->setColumnWidth(7, 100);
    m_issTable->setColumnWidth(8, 80);
    m_issTable->setColumnWidth(9, 100);
    m_issTable->setColumnWidth(10, 100);
    m_issTable->horizontalHeader()->setStretchLastSection(true);

    layout->addWidget(m_issTable, 1);
    return tab;
}

void UniformWidget::loadInventory()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute("SELECT * FROM Uniform ORDER BY item_type, size");

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM Uniform");
    if (cc.next()) count = cc.value(0).toInt();
    m_invTable->setRowCount(count);
    m_invTable->setSortingEnabled(false);

    int row = 0;
    int totalQty = 0;
    int availableCount = 0;

    while (query.next()) {
        auto setItem = [&](int col, const QString& text, const QColor& fg = QColor()) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            if (fg.isValid()) item->setForeground(QBrush(fg));
            m_invTable->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        setItem(1, query.value("item_type").toString());

        auto* sizeItem = new QTableWidgetItem(query.value("size").toString());
        sizeItem->setTextAlignment(Qt::AlignCenter);
        m_invTable->setItem(row, 2, sizeItem);

        int qty = query.value("quantity").toInt();
        totalQty += qty;

        auto* qtyItem = new QTableWidgetItem(QString::number(qty));
        qtyItem->setTextAlignment(Qt::AlignCenter);
        QColor qtyColor = qty <= 0 ? QColor("#E85454") : (qty <= 3 ? QColor("#FBBF24") : QColor("#4ADE80"));
        qtyItem->setForeground(QBrush(qtyColor));
        m_invTable->setItem(row, 3, qtyItem);

        QString status = query.value("status").toString();
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "Available")      statusItem->setForeground(QColor("#4ADE80"));
        else if (status == "Issued")    statusItem->setForeground(QColor("#FBBF24"));
        else if (status == "Damaged")   statusItem->setForeground(QColor("#FB923C"));
        else if (status == "Retired")   statusItem->setForeground(QColor("#6B7585"));
        if (status == "Available") availableCount++;
        m_invTable->setItem(row, 4, statusItem);

        setItem(5, query.value("notes").toString());
        setItem(6, query.value("created_at").toString());

        row++;
    }

    m_invTable->setSortingEnabled(true);
    m_invSummary->setText(
        QString("Items: %1 | Total Qty: %2 | Available: %3")
            .arg(row).arg(totalQty).arg(availableCount)
    );
}

void UniformWidget::loadIssuance()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT ui.*, g.guard_code, g.full_name, u.item_type, u.size "
        "FROM UniformIssue ui "
        "JOIN Guards g ON ui.guard_id = g.id "
        "JOIN Uniform u ON ui.uniform_id = u.id "
        "ORDER BY ui.created_at DESC"
    );

    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM UniformIssue");
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
        setItem(1, query.value("uniform_id").toString());
        setItem(2, query.value("guard_code").toString());
        setItem(3, query.value("full_name").toString());
        setItem(4, query.value("item_type").toString());

        auto* sizeItem = new QTableWidgetItem(query.value("size").toString());
        sizeItem->setTextAlignment(Qt::AlignCenter);
        m_issTable->setItem(row, 5, sizeItem);

        setItem(6, query.value("issue_date").toString());
        setItem(7, query.value("condition_out").toString());

        // Status
        auto* statusItem = new QTableWidgetItem(isReturned ? "Returned" : "Issued");
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (isReturned) {
            statusItem->setForeground(QColor("#4ADE80"));
            returnedCount++;
        } else {
            statusItem->setForeground(QColor("#FBBF24"));
            issuedCount++;
        }
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

void UniformWidget::refresh()
{
    loadInventory();
    loadIssuance();
}

void UniformWidget::addItem()
{
    UniformDialog dlg(UniformDialog::ItemMode, this, -1);
    if (dlg.exec() == QDialog::Accepted) loadInventory();
}

void UniformWidget::editItem()
{
    auto items = m_invTable->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Select an item to edit.");
        return;
    }
    int id = m_invTable->item(items.first()->row(), 0)->text().toInt();
    UniformDialog dlg(UniformDialog::ItemMode, this, id);
    if (dlg.exec() == QDialog::Accepted) loadInventory();
}

void UniformWidget::deleteItem()
{
    auto items = m_invTable->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Select an item to delete.");
        return;
    }
    int row = items.first()->row();
    int id = m_invTable->item(row, 0)->text().toInt();
    QString name = m_invTable->item(row, 1)->text();

    auto result = QMessageBox::question(this, "Delete",
        QString("Delete \"%1\" from inventory?").arg(name),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("DELETE FROM Uniform WHERE id = :id", {{":id", id}});
        loadInventory();
    }
}

void UniformWidget::issueUniform()
{
    UniformDialog dlg(UniformDialog::IssueMode, this);
    if (dlg.exec() == QDialog::Accepted) {
        loadInventory();
        loadIssuance();
    }
}

void UniformWidget::returnUniform()
{
    auto items = m_issTable->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::information(this, "No Selection",
            "Select an issuance record to return.");
        return;
    }
    int row = items.first()->row();
    int issueId = m_issTable->item(row, 0)->text().toInt();
    QString status = m_issTable->item(row, 8)->text();

    if (status == "Returned") {
        QMessageBox::information(this, "Already Returned",
            "This uniform has already been returned.");
        return;
    }

    UniformDialog dlg(UniformDialog::ReturnMode, this, issueId);
    if (dlg.exec() == QDialog::Accepted) {
        loadInventory();
        loadIssuance();
    }
}

void UniformWidget::filterIssuance(const QString& text)
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

void UniformWidget::exportCSV()
{
    if (m_issTable->rowCount() == 0) {
        QMessageBox::information(this, "No Data", "No issuance data to export.");
        return;
    }

    QDir().mkpath(QCoreApplication::applicationDirPath() + "/reports");
    QString filePath = QFileDialog::getSaveFileName(
        this, "Export Uniform Report",
        QCoreApplication::applicationDirPath() + "/reports/uniform_report.csv",
        "CSV Files (*.csv);;All Files (*)"
    );
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file.");
        return;
    }

    QTextStream out(&file);

    // Visible columns only
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
        QString("Uniform report exported to:\n\n%1").arg(filePath));
}
