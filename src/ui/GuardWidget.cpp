#include "GuardWidget.h"
#include "GuardDialog.h"
#include "database/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollArea>
#include <QFrame>

GuardWidget::GuardWidget(QWidget* parent)
    : QWidget(parent)
{
    buildUI();
    loadGuards();
}

void GuardWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    // ---- Header ----
    auto* headerRow = new QHBoxLayout;

    auto* leftHeader = new QVBoxLayout;
    auto* titleLabel = new QLabel("Guard Management");
    titleLabel->setObjectName("PageTitle");
    leftHeader->addWidget(titleLabel);

    m_countLabel = new QLabel("0 guards");
    m_countLabel->setObjectName("PageSubtitle");
    leftHeader->addWidget(m_countLabel);

    headerRow->addLayout(leftHeader);
    headerRow->addStretch();

    m_addBtn = new QPushButton("+ Add Guard");
    m_addBtn->setObjectName("PrimaryButton");
    m_addBtn->setFixedSize(140, 40);
    m_addBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addBtn, &QPushButton::clicked, this, &GuardWidget::addGuard);
    headerRow->addWidget(m_addBtn);

    mainLayout->addLayout(headerRow);

    // ---- Search / Filter bar ----
    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by name, code, mobile, Aadhaar...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &GuardWidget::filterGuards);
    filterRow->addWidget(m_searchEdit, 1);

    m_statusFilter = new QComboBox;
    m_statusFilter->addItems({"All Status", "Active", "Inactive", "On Leave", "Terminated"});
    m_statusFilter->setFixedWidth(150);
    connect(m_statusFilter, &QComboBox::currentTextChanged, this,
            [this](const QString&) { filterGuards(m_searchEdit->text()); });
    filterRow->addWidget(m_statusFilter);

    mainLayout->addLayout(filterRow);

    // ---- Action buttons ----
    auto* actionRow = new QHBoxLayout;
    actionRow->setSpacing(8);

    m_editBtn = new QPushButton("Edit");
    m_editBtn->setObjectName("SecondaryButton");
    m_editBtn->setFixedSize(80, 36);
    m_editBtn->setCursor(Qt::PointingHandCursor);
    connect(m_editBtn, &QPushButton::clicked, this, &GuardWidget::editGuard);

    m_viewBtn = new QPushButton("View Profile");
    m_viewBtn->setObjectName("SecondaryButton");
    m_viewBtn->setFixedSize(120, 36);
    m_viewBtn->setCursor(Qt::PointingHandCursor);
    connect(m_viewBtn, &QPushButton::clicked, this, &GuardWidget::viewProfile);

    m_deleteBtn = new QPushButton("Delete");
    m_deleteBtn->setObjectName("DangerButton");
    m_deleteBtn->setFixedSize(90, 36);
    m_deleteBtn->setCursor(Qt::PointingHandCursor);
    connect(m_deleteBtn, &QPushButton::clicked, this, &GuardWidget::deleteGuard);

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
        "ID", "Code", "Name", "Mobile", "Gender", "Aadhaar",
        "Joining Date", "Status", "Client", "Site"
    };
    m_table->setColumnCount(columns.size());
    m_table->setHorizontalHeaderLabels(columns);

    auto* header = m_table->horizontalHeader();
    header->setStretchLastSection(true);
    header->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->setColumnHidden(0, true);

    m_table->setColumnWidth(1, 90);
    m_table->setColumnWidth(2, 200);
    m_table->setColumnWidth(3, 120);
    m_table->setColumnWidth(4, 80);
    m_table->setColumnWidth(5, 130);
    m_table->setColumnWidth(6, 110);
    m_table->setColumnWidth(7, 90);
    m_table->setColumnWidth(8, 150);

    connect(m_table, &QTableWidget::cellDoubleClicked, this, &GuardWidget::editGuard);

    mainLayout->addWidget(m_table, 1);
}

void GuardWidget::loadGuards()
{
    auto guards = DatabaseManager::instance().getAllGuards();

    m_table->setSortingEnabled(false);
    m_table->setRowCount(guards.size());

    for (int row = 0; row < guards.size(); ++row) {
        const auto& g = guards[row].toMap();

        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };

        setItem(0, g["id"].toString());
        setItem(1, g["guard_code"].toString());
        setItem(2, g["full_name"].toString());
        setItem(3, g["mobile_primary"].toString());
        setItem(4, g["gender"].toString());
        setItem(5, g["aadhaar_number"].toString());
        setItem(6, g["joining_date"].toString());
        setItem(7, g["status"].toString());
        setItem(8, g["client_name"].toString());
        setItem(9, g["site_name"].toString());

        auto* statusItem = m_table->item(row, 7);
        QString status = g["status"].toString();
        if (status == "Active") {
            statusItem->setForeground(QColor("#4ADE80"));
        } else if (status == "Inactive" || status == "Terminated") {
            statusItem->setForeground(QColor("#E85454"));
        } else if (status == "On Leave") {
            statusItem->setForeground(QColor("#FBBF24"));
        }
    }

    m_table->setSortingEnabled(true);
    m_countLabel->setText(QString("%1 guards").arg(guards.size()));
}

void GuardWidget::refresh()
{
    loadGuards();
}

int GuardWidget::selectedGuardId() const
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) return -1;
    int row = items.first()->row();
    return m_table->item(row, 0)->text().toInt();
}

void GuardWidget::addGuard()
{
    GuardDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        loadGuards();
    }
}

void GuardWidget::editGuard()
{
    int id = selectedGuardId();
    if (id < 0) {
        QMessageBox::information(this, "No Selection", "Please select a guard to edit.");
        return;
    }

    GuardDialog dlg(this, id);
    if (dlg.exec() == QDialog::Accepted) {
        loadGuards();
    }
}

void GuardWidget::deleteGuard()
{
    int id = selectedGuardId();
    if (id < 0) {
        QMessageBox::information(this, "No Selection", "Please select a guard to delete.");
        return;
    }

    int row = m_table->currentRow();
    QString name = m_table->item(row, 2)->text();

    auto result = QMessageBox::question(
        this, "Confirm Delete",
        QString("Are you sure you want to delete guard \"%1\"?\n\n"
                "This action cannot be undone.").arg(name),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (result == QMessageBox::Yes) {
        if (DatabaseManager::instance().deleteGuard(id)) {
            loadGuards();
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete guard.");
        }
    }
}

void GuardWidget::viewProfile()
{
    int id = selectedGuardId();
    if (id < 0) {
        QMessageBox::information(this, "No Selection", "Please select a guard to view.");
        return;
    }

    auto data = DatabaseManager::instance().getGuardById(id);
    if (data.isEmpty()) return;

    QString info = QString(
        "Guard Profile\n"
        "==============================================\n"
        "Code:           %1\n"
        "Name:           %2\n"
        "Father:         %3\n"
        "DOB:            %4\n"
        "Gender:         %5\n"
        "Mobile:         %6\n"
        "Aadhaar:        %7\n"
        "PAN:            %8\n"
        "==============================================\n"
        "Joining Date:   %9\n"
        "Status:         %10\n"
        "==============================================\n"
        "Bank:           %11\n"
        "Account:        %12\n"
        "IFSC:           %13\n"
        "==============================================\n"
        "Basic Salary:   Rs. %14\n"
        "Client:         %15\n"
        "Site:           %16"
    ).arg(
        data["guard_code"].toString(),
        data["full_name"].toString(),
        data["father_name"].toString(),
        data["date_of_birth"].toString(),
        data["gender"].toString(),
        data["mobile_primary"].toString(),
        data["aadhaar_number"].toString(),
        data["pan_number"].toString(),
        data["joining_date"].toString(),
        data["status"].toString(),
        data["bank_name"].toString(),
        data["bank_account"].toString(),
        data["bank_ifsc"].toString(),
        QString::number(data["basic_salary"].toDouble(), 'f', 0),
        data["client_name"].toString(),
        data["site_name"].toString()
    );

    QMessageBox profileBox(this);
    profileBox.setWindowTitle("Guard Profile - " + data["full_name"].toString());
    profileBox.setText(info);
    profileBox.setFont(QFont("Consolas", 10));
    profileBox.exec();
}

void GuardWidget::filterGuards(const QString& text)
{
    QString searchText = text.toLower();
    QString statusFilter = m_statusFilter->currentText();

    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool textMatch = searchText.isEmpty();
        bool statusMatch = (statusFilter == "All Status");

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
            auto* statusItem = m_table->item(row, 7);
            if (statusItem) {
                statusMatch = (statusItem->text() == statusFilter);
            }
        }

        m_table->setRowHidden(row, !(textMatch && statusMatch));
    }
}
