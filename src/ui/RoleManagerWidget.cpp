#include "RoleManagerWidget.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QCheckBox>
#include <QGroupBox>

RoleManagerWidget::RoleManagerWidget(QWidget* parent) : QWidget(parent) { buildUI(); loadPermissions(); }

void RoleManagerWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Role-Based Access Control");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);
    auto* subtitle = new QLabel("Configure module access permissions for each role");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(10);

    auto* roleLbl = new QLabel("Role:");
    roleLbl->setStyleSheet("color: #8B95A5; font-weight: 600; font-size: 14px;");
    headerRow->addWidget(roleLbl);

    m_roleCombo = new QComboBox;
    m_roleCombo->addItems({"Admin", "Supervisor", "Operator"});
    m_roleCombo->setFixedWidth(200);
    connect(m_roleCombo, &QComboBox::currentTextChanged, this, &RoleManagerWidget::roleChanged);
    headerRow->addWidget(m_roleCombo);

    headerRow->addSpacing(20);

    m_infoLabel = new QLabel;
    m_infoLabel->setStyleSheet("color: #60A5FA; font-size: 13px; font-style: italic;");
    headerRow->addWidget(m_infoLabel);

    headerRow->addStretch();

    auto* saveBtn = new QPushButton("Save Changes");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(140, 36);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &RoleManagerWidget::savePermissions);
    headerRow->addWidget(saveBtn);

    mainLayout->addLayout(headerRow);

    m_table = new QTableWidget;
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);

    QStringList cols = {"Module", "View", "Create", "Edit", "Delete"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnWidth(0, 250);
    m_table->setColumnWidth(1, 100);
    m_table->setColumnWidth(2, 100);
    m_table->setColumnWidth(3, 100);
    m_table->horizontalHeader()->setStretchLastSection(true);
    mainLayout->addWidget(m_table, 1);

    auto* descGroup = new QGroupBox("Role Descriptions");
    auto* descLayout = new QVBoxLayout(descGroup);
    auto addDesc = [&](const QString& role, const QString& desc) {
        auto* lbl = new QLabel(QString("<b>%1:</b> %2").arg(role, desc));
        lbl->setStyleSheet("color: #8B95A5; font-size: 12px; padding: 2px 0;");
        lbl->setTextFormat(Qt::RichText);
        descLayout->addWidget(lbl);
    };
    addDesc("Admin", "Full system access. Can manage all modules, settings, backups, and permissions.");
    addDesc("Supervisor", "Operational management. Can manage guards, attendance, duty, salary. No access to Settings, Backup, or Audit Log.");
    addDesc("Operator", "Basic operations. Can mark attendance, register visitors, log incidents, and submit leave requests. Read-only for most modules.");
    mainLayout->addWidget(descGroup);
}

void RoleManagerWidget::roleChanged(const QString& role)
{
    if (role == "Admin") {
        m_infoLabel->setText("Admin has full access. Changes affect Supervisor and Operator roles only.");
    } else {
        m_infoLabel->clear();
    }
    loadPermissions();
}

void RoleManagerWidget::loadPermissions()
{
    QString role = m_roleCombo->currentText();
    auto& db = DatabaseManager::instance();
    auto query = db.getPermissions(role);

    QStringList allModules = {"Dashboard", "Guards", "Clients", "Sites", "Attendance",
                              "Duty", "Leave", "Salary", "Uniform", "Equipment",
                              "Visitors", "Vehicles", "Incidents", "Training",
                              "Documents", "Complaints", "Fines", "Alerts",
                              "Payroll", "Announcements", "Photos", "Invoices",
                              "Tickets", "AuditLog", "Reports", "Search",
                              "Backup", "Settings"};

    QMap<QString, QHash<QString, bool>> perms;
    while (query.next()) {
        QString mod = query.value("module_name").toString();
        perms[mod]["can_view"] = query.value("can_view").toInt() == 1;
        perms[mod]["can_create"] = query.value("can_create").toInt() == 1;
        perms[mod]["can_edit"] = query.value("can_edit").toInt() == 1;
        perms[mod]["can_delete"] = query.value("can_delete").toInt() == 1;
    }

    m_table->setRowCount(allModules.size());
    for (int row = 0; row < allModules.size(); ++row) {
        QString mod = allModules[row];
        auto* nameItem = new QTableWidgetItem(mod);
        nameItem->setForeground(QColor("#D4B44C"));
        m_table->setItem(row, 0, nameItem);

        bool view = perms[mod].value("can_view", true);
        bool create = perms[mod].value("can_create", true);
        bool edit = perms[mod].value("can_edit", true);
        bool del = perms[mod].value("can_delete", false);

        auto makeCheck = [&](int col, bool checked) {
            auto* check = new QTableWidgetItem(checked ? "Yes" : "No");
            check->setTextAlignment(Qt::AlignCenter);
            check->setForeground(checked ? QColor("#4ADE80") : QColor("#E85454"));
            check->setData(Qt::UserRole, checked ? 1 : 0);
            m_table->setItem(row, col, check);
        };
        makeCheck(1, view);
        makeCheck(2, create);
        makeCheck(3, edit);
        makeCheck(4, del);
    }
}

void RoleManagerWidget::savePermissions()
{
    QString role = m_roleCombo->currentText();
    auto& db = DatabaseManager::instance();

    for (int row = 0; row < m_table->rowCount(); ++row) {
        QString module = m_table->item(row, 0)->text();
        int canView = m_table->item(row, 1)->data(Qt::UserRole).toInt();
        int canCreate = m_table->item(row, 2)->data(Qt::UserRole).toInt();
        int canEdit = m_table->item(row, 3)->data(Qt::UserRole).toInt();
        int canDelete = m_table->item(row, 4)->data(Qt::UserRole).toInt();

        db.executeNonQuery(
            "UPDATE RolePermissions SET can_view=:v, can_create=:c, can_edit=:e, can_delete=:d "
            "WHERE role_name=:role AND module_name=:mod",
            {{":v", canView}, {":c", canCreate}, {":e", canEdit}, {":d", canDelete},
             {":role", role}, {":mod", module}});
    }

    QMessageBox::information(this, "Saved", QString("Permissions for '%1' role updated successfully.\n\nChanges take effect on next login.").arg(role));
}

void RoleManagerWidget::refresh() { loadPermissions(); }
