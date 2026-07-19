#include "ClientWidget.h"
#include "ClientDialog.h"
#include "database/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFrame>

ClientWidget::ClientWidget(QWidget* parent)
    : QWidget(parent)
{
    buildUI();
    loadClients();
}

void ClientWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    // ---- Header ----
    auto* headerRow = new QHBoxLayout;

    auto* leftHeader = new QVBoxLayout;
    auto* titleLabel = new QLabel("Client Management");
    titleLabel->setObjectName("PageTitle");
    leftHeader->addWidget(titleLabel);

    m_countLabel = new QLabel("0 clients");
    m_countLabel->setObjectName("PageSubtitle");
    leftHeader->addWidget(m_countLabel);

    headerRow->addLayout(leftHeader);
    headerRow->addStretch();

    m_addBtn = new QPushButton("+ Add Client");
    m_addBtn->setObjectName("PrimaryButton");
    m_addBtn->setFixedSize(150, 40);
    m_addBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addBtn, &QPushButton::clicked, this, &ClientWidget::addClient);
    headerRow->addWidget(m_addBtn);

    mainLayout->addLayout(headerRow);

    // ---- Search / Filter ----
    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by name, code, contact, mobile, GST...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ClientWidget::filterClients);
    filterRow->addWidget(m_searchEdit, 1);

    m_statusFilter = new QComboBox;
    m_statusFilter->addItems({
        "All Status", "Active", "Inactive",
        "Under Negotiation", "Contract Expired"
    });
    m_statusFilter->setFixedWidth(180);
    connect(m_statusFilter, &QComboBox::currentTextChanged, this,
            [this](const QString&) { filterClients(m_searchEdit->text()); });
    filterRow->addWidget(m_statusFilter);

    mainLayout->addLayout(filterRow);

    // ---- Action buttons ----
    auto* actionRow = new QHBoxLayout;
    actionRow->setSpacing(8);

    m_editBtn = new QPushButton("Edit");
    m_editBtn->setObjectName("SecondaryButton");
    m_editBtn->setFixedSize(80, 36);
    m_editBtn->setCursor(Qt::PointingHandCursor);
    connect(m_editBtn, &QPushButton::clicked, this, &ClientWidget::editClient);

    m_viewBtn = new QPushButton("View Details");
    m_viewBtn->setObjectName("SecondaryButton");
    m_viewBtn->setFixedSize(120, 36);
    m_viewBtn->setCursor(Qt::PointingHandCursor);
    connect(m_viewBtn, &QPushButton::clicked, this, &ClientWidget::viewClient);

    m_deleteBtn = new QPushButton("Delete");
    m_deleteBtn->setObjectName("DangerButton");
    m_deleteBtn->setFixedSize(90, 36);
    m_deleteBtn->setCursor(Qt::PointingHandCursor);
    connect(m_deleteBtn, &QPushButton::clicked, this, &ClientWidget::deleteClient);

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
        "ID", "Code", "Company Name", "Contact Person", "Mobile",
        "City", "GST", "Billing Rate", "Contract Start", "Contract End", "Status"
    };
    m_table->setColumnCount(columns.size());
    m_table->setHorizontalHeaderLabels(columns);
    m_table->setColumnHidden(0, true);

    auto* header = m_table->horizontalHeader();
    header->setStretchLastSection(true);
    header->setSectionResizeMode(2, QHeaderView::Stretch);

    m_table->setColumnWidth(1, 80);
    m_table->setColumnWidth(2, 200);
    m_table->setColumnWidth(3, 140);
    m_table->setColumnWidth(4, 120);
    m_table->setColumnWidth(5, 100);
    m_table->setColumnWidth(6, 140);
    m_table->setColumnWidth(7, 110);
    m_table->setColumnWidth(8, 110);
    m_table->setColumnWidth(9, 110);
    m_table->setColumnWidth(10, 100);

    connect(m_table, &QTableWidget::cellDoubleClicked, this, &ClientWidget::editClient);

    mainLayout->addWidget(m_table, 1);
}

void ClientWidget::loadClients()
{
    auto& db = DatabaseManager::instance();
    QSqlQuery query = db.execute(
        "SELECT * FROM Clients ORDER BY company_name"
    );

    m_table->setSortingEnabled(false);
    int row = 0;

    // Count rows first
    int count = 0;
    while (query.next()) count++;
    query.first();
    query.previous();
    m_table->setRowCount(count);

    query.exec("SELECT * FROM Clients ORDER BY company_name");

    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        setItem(1, query.value("client_code").toString());
        setItem(2, query.value("company_name").toString());
        setItem(3, query.value("contact_person").toString());
        setItem(4, query.value("mobile").toString());
        setItem(5, query.value("city").toString());
        setItem(6, query.value("gst_number").toString());

        double rate = query.value("billing_rate").toDouble();
        setItem(7, rate > 0 ? QString("Rs. %1").arg(rate, 0, 'f', 0) : "-");

        setItem(8, query.value("contract_start").toString());
        setItem(9, query.value("contract_end").toString());
        setItem(10, query.value("status").toString());

        // Color-code status
        auto* statusItem = m_table->item(row, 10);
        QString status = query.value("status").toString();
        if (status == "Active") {
            statusItem->setForeground(QColor("#4ADE80"));
        } else if (status == "Inactive" || status == "Contract Expired") {
            statusItem->setForeground(QColor("#E85454"));
        } else if (status == "Under Negotiation") {
            statusItem->setForeground(QColor("#FBBF24"));
        }

        row++;
    }

    m_table->setSortingEnabled(true);
    m_countLabel->setText(QString("%1 clients").arg(row));
}

void ClientWidget::refresh()
{
    loadClients();
}

int ClientWidget::selectedClientId() const
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) return -1;
    int row = items.first()->row();
    return m_table->item(row, 0)->text().toInt();
}

void ClientWidget::addClient()
{
    ClientDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        loadClients();
    }
}

void ClientWidget::editClient()
{
    int id = selectedClientId();
    if (id < 0) {
        QMessageBox::information(this, "No Selection", "Please select a client to edit.");
        return;
    }
    ClientDialog dlg(this, id);
    if (dlg.exec() == QDialog::Accepted) {
        loadClients();
    }
}

void ClientWidget::deleteClient()
{
    int id = selectedClientId();
    if (id < 0) {
        QMessageBox::information(this, "No Selection", "Please select a client to delete.");
        return;
    }

    int row = m_table->currentRow();
    QString name = m_table->item(row, 2)->text();

    auto result = QMessageBox::question(
        this, "Confirm Delete",
        QString("Are you sure you want to delete client \"%1\"?\n\n"
                "This will also affect related sites and guard assignments.").arg(name),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        if (db.executeNonQuery("DELETE FROM Clients WHERE id = :id", {{":id", id}})) {
            loadClients();
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete client.");
        }
    }
}

void ClientWidget::viewClient()
{
    int id = selectedClientId();
    if (id < 0) {
        QMessageBox::information(this, "No Selection", "Please select a client to view.");
        return;
    }

    auto& db = DatabaseManager::instance();
    QSqlQuery query = db.execute("SELECT * FROM Clients WHERE id = :id", {{":id", id}});
    if (!query.next()) return;

    // Count sites and guards for this client
    QSqlQuery siteQuery = db.execute(
        "SELECT COUNT(*) FROM Sites WHERE client_id = :id", {{":id", id}}
    );
    int siteCount = siteQuery.next() ? siteQuery.value(0).toInt() : 0;

    QSqlQuery guardQuery = db.execute(
        "SELECT COUNT(*) FROM Guards WHERE client_id = :id AND status = 'Active'", {{":id", id}}
    );
    int guardCount = guardQuery.next() ? guardQuery.value(0).toInt() : 0;

    QSqlQuery salaryQuery = db.execute(
        "SELECT COALESCE(SUM(net_salary),0) FROM Salary "
        "WHERE guard_id IN (SELECT id FROM Guards WHERE client_id = :id) "
        "AND payment_status = 'Pending'",
        {{":id", id}}
    );
    double pendingAmount = salaryQuery.next() ? salaryQuery.value(0).toDouble() : 0;

    QString info = QString(
        "Client Details\n"
        "============================================\n"
        "Code:            %1\n"
        "Company:         %2\n"
        "Contact Person:  %3\n"
        "Mobile:          %4\n"
        "Email:           %5\n"
        "Address:         %6\n"
        "City:            %7\n"
        "GST:             %8\n"
        "============================================\n"
        "Billing Rate:    Rs. %9 / guard / month\n"
        "Contract Start:  %10\n"
        "Contract End:    %11\n"
        "Status:          %12\n"
        "============================================\n"
        "Sites:           %13\n"
        "Active Guards:   %14\n"
        "Pending Salary:  Rs. %15\n"
        "============================================\n"
        "Invoice Terms:   %16\n"
        "Notes:           %17"
    ).arg(
        query.value("client_code").toString(),
        query.value("company_name").toString(),
        query.value("contact_person").toString(),
        query.value("mobile").toString(),
        query.value("email").toString(),
        query.value("address").toString(),
        query.value("city").toString(),
        query.value("gst_number").toString(),
        QString::number(query.value("billing_rate").toDouble(), 'f', 0),
        query.value("contract_start").toString(),
        query.value("contract_end").toString(),
        query.value("status").toString(),
        QString::number(siteCount),
        QString::number(guardCount),
        QString::number(pendingAmount, 'f', 0),
        query.value("invoice_terms").toString(),
        query.value("notes").toString()
    );

    QMessageBox box(this);
    box.setWindowTitle("Client — " + query.value("company_name").toString());
    box.setText(info);
    box.setFont(QFont("Consolas", 10));
    box.exec();
}

void ClientWidget::filterClients(const QString& text)
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
            auto* statusItem = m_table->item(row, 10);
            if (statusItem) {
                statusMatch = (statusItem->text() == statusFilter);
            }
        }

        m_table->setRowHidden(row, !(textMatch && statusMatch));
    }
}
