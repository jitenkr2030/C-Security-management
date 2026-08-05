#include "SearchWidget.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

SearchWidget::SearchWidget(QWidget* parent) : QWidget(parent)
{
    buildUI();
}

void SearchWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Global Search");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);
    auto* subtitle = new QLabel("Search across all modules in the database");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    // Search controls
    auto* searchRow = new QHBoxLayout;
    searchRow->setSpacing(10);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Type to search...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &SearchWidget::performSearch);
    searchRow->addWidget(m_searchEdit, 1);

    m_entityCombo = new QComboBox;
    m_entityCombo->addItems({"All", "Guards", "Clients", "Sites", "Visitors", "Vehicles", "Incidents"});
    m_entityCombo->setFixedWidth(140);
    searchRow->addWidget(m_entityCombo);

    auto* searchBtn = new QPushButton("Search");
    searchBtn->setObjectName("PrimaryButton");
    searchBtn->setFixedSize(100, 36);
    searchBtn->setCursor(Qt::PointingHandCursor);
    connect(searchBtn, &QPushButton::clicked, this, &SearchWidget::performSearch);
    searchRow->addWidget(searchBtn);

    mainLayout->addLayout(searchRow);

    // Result label
    m_resultLabel = new QLabel;
    m_resultLabel->setStyleSheet("color: #D4B44C; font-size: 13px; font-weight: 600;");
    mainLayout->addWidget(m_resultLabel);

    // Table
    m_table = new QTableWidget;
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(true);

    QStringList cols = {"Module", "ID", "Code/Name", "Details", "Status"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(1, true);
    m_table->setColumnWidth(0, 120);
    m_table->setColumnWidth(2, 200);
    m_table->setColumnWidth(3, 400);
    m_table->horizontalHeader()->setStretchLastSection(true);

    mainLayout->addWidget(m_table, 1);
}

void SearchWidget::refresh() {}

void SearchWidget::performSearch()
{
    QString text = m_searchEdit->text().trimmed();
    if (text.isEmpty()) {
        m_resultLabel->setText("Please enter a search term.");
        return;
    }

    m_table->setSortingEnabled(false);
    m_table->setRowCount(0);

    QString entity = m_entityCombo->currentText();
    int totalResults = 0;

    if (entity == "All" || entity == "Guards") { searchGuards(text); totalResults += m_table->rowCount(); }
    if (entity == "All" || entity == "Clients") { searchClients(text); totalResults += m_table->rowCount(); }
    if (entity == "All" || entity == "Sites") { searchSites(text); totalResults += m_table->rowCount(); }
    if (entity == "All" || entity == "Visitors") { searchVisitors(text); totalResults += m_table->rowCount(); }
    if (entity == "All" || entity == "Vehicles") { searchVehicles(text); totalResults += m_table->rowCount(); }
    if (entity == "All" || entity == "Incidents") { searchIncidents(text); totalResults += m_table->rowCount(); }

    m_table->setSortingEnabled(true);
    m_resultLabel->setText(QString("Found %1 results for \"%2\"").arg(m_table->rowCount()).arg(text));
}

void SearchWidget::openResult()
{
    // Future: navigate to the specific module and item
}

void SearchWidget::searchGuards(const QString& text)
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT g.*, s.site_name FROM Guards g LEFT JOIN Sites s ON g.site_id = s.id "
        "WHERE g.full_name LIKE :t OR g.guard_code LIKE :t OR g.mobile LIKE :t OR g.aadhaar LIKE :t "
        "ORDER BY g.full_name LIMIT 50",
        {{":t", "%" + text + "%"}}
    );

    while (query.next()) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        auto* modItem = new QTableWidgetItem("Guard");
        modItem->setForeground(QColor("#4ADE80"));
        m_table->setItem(row, 0, modItem);
        m_table->setItem(row, 1, new QTableWidgetItem(query.value("id").toString()));
        m_table->setItem(row, 2, new QTableWidgetItem(
            query.value("guard_code").toString() + " - " + query.value("full_name").toString()));
        m_table->setItem(row, 3, new QTableWidgetItem(
            "Mobile: " + query.value("mobile").toString() + " | Site: " + query.value("site_name").toString()));
        m_table->setItem(row, 4, new QTableWidgetItem(query.value("status").toString()));
    }
}

void SearchWidget::searchClients(const QString& text)
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT * FROM Clients WHERE client_name LIKE :t OR contact_person LIKE :t OR mobile LIKE :t "
        "ORDER BY client_name LIMIT 50",
        {{":t", "%" + text + "%"}}
    );

    while (query.next()) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        auto* modItem = new QTableWidgetItem("Client");
        modItem->setForeground(QColor("#60A5FA"));
        m_table->setItem(row, 0, modItem);
        m_table->setItem(row, 1, new QTableWidgetItem(query.value("id").toString()));
        m_table->setItem(row, 2, new QTableWidgetItem(query.value("client_name").toString()));
        m_table->setItem(row, 3, new QTableWidgetItem(
            "Contact: " + query.value("contact_person").toString() + " | " + query.value("mobile").toString()));
        m_table->setItem(row, 4, new QTableWidgetItem(query.value("status").toString()));
    }
}

void SearchWidget::searchSites(const QString& text)
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT s.*, c.client_name FROM Sites s LEFT JOIN Clients c ON s.client_id = c.id "
        "WHERE s.site_name LIKE :t OR s.address LIKE :t OR s.city LIKE :t "
        "ORDER BY s.site_name LIMIT 50",
        {{":t", "%" + text + "%"}}
    );

    while (query.next()) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        auto* modItem = new QTableWidgetItem("Site");
        modItem->setForeground(QColor("#FBBF24"));
        m_table->setItem(row, 0, modItem);
        m_table->setItem(row, 1, new QTableWidgetItem(query.value("id").toString()));
        m_table->setItem(row, 2, new QTableWidgetItem(query.value("site_name").toString()));
        m_table->setItem(row, 3, new QTableWidgetItem(
            query.value("address").toString() + ", " + query.value("city").toString()
            + " | Client: " + query.value("client_name").toString()));
        m_table->setItem(row, 4, new QTableWidgetItem(query.value("status").toString()));
    }
}

void SearchWidget::searchVisitors(const QString& text)
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT v.*, s.site_name FROM Visitors v JOIN Sites s ON v.site_id = s.id "
        "WHERE v.name LIKE :t OR v.mobile LIKE :t OR v.vehicle_no LIKE :t "
        "ORDER BY v.entry_time DESC LIMIT 50",
        {{":t", "%" + text + "%"}}
    );

    while (query.next()) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        auto* modItem = new QTableWidgetItem("Visitor");
        modItem->setForeground(QColor("#A78BFA"));
        m_table->setItem(row, 0, modItem);
        m_table->setItem(row, 1, new QTableWidgetItem(query.value("id").toString()));
        m_table->setItem(row, 2, new QTableWidgetItem(query.value("name").toString()));
        m_table->setItem(row, 3, new QTableWidgetItem(
            "Site: " + query.value("site_name").toString()
            + " | Purpose: " + query.value("purpose").toString()
            + " | Entry: " + query.value("entry_time").toString()));
        m_table->setItem(row, 4, new QTableWidgetItem(
            query.value("exit_time").toString().isEmpty() ? "Inside" : "Exited"));
    }
}

void SearchWidget::searchVehicles(const QString& text)
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT v.*, s.site_name FROM Vehicles v JOIN Sites s ON v.site_id = s.id "
        "WHERE v.vehicle_no LIKE :t OR v.driver_name LIKE :t OR v.driver_mobile LIKE :t "
        "ORDER BY v.entry_time DESC LIMIT 50",
        {{":t", "%" + text + "%"}}
    );

    while (query.next()) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        auto* modItem = new QTableWidgetItem("Vehicle");
        modItem->setForeground(QColor("#FB923C"));
        m_table->setItem(row, 0, modItem);
        m_table->setItem(row, 1, new QTableWidgetItem(query.value("id").toString()));
        m_table->setItem(row, 2, new QTableWidgetItem(query.value("vehicle_no").toString()));
        m_table->setItem(row, 3, new QTableWidgetItem(
            "Driver: " + query.value("driver_name").toString()
            + " | Site: " + query.value("site_name").toString()
            + " | Entry: " + query.value("entry_time").toString()));
        m_table->setItem(row, 4, new QTableWidgetItem(
            query.value("exit_time").toString().isEmpty() ? "Inside" : "Exited"));
    }
}

void SearchWidget::searchIncidents(const QString& text)
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute(
        "SELECT i.*, s.site_name FROM Incidents i LEFT JOIN Sites s ON i.site_id = s.id "
        "WHERE i.incident_code LIKE :t OR i.incident_type LIKE :t OR i.description LIKE :t "
        "ORDER BY i.date_time DESC LIMIT 50",
        {{":t", "%" + text + "%"}}
    );

    while (query.next()) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        auto* modItem = new QTableWidgetItem("Incident");
        modItem->setForeground(QColor("#E85454"));
        m_table->setItem(row, 0, modItem);
        m_table->setItem(row, 1, new QTableWidgetItem(query.value("id").toString()));
        m_table->setItem(row, 2, new QTableWidgetItem(
            query.value("incident_code").toString() + " - " + query.value("incident_type").toString()));
        m_table->setItem(row, 3, new QTableWidgetItem(
            query.value("description").toString().left(100)
            + " | Severity: " + query.value("severity").toString()));
        m_table->setItem(row, 4, new QTableWidgetItem(query.value("status").toString()));
    }
}
