#include "MainWindow.h"
#include "DashboardWidget.h"
#include "GuardWidget.h"
#include "ClientWidget.h"
#include "SiteWidget.h"
#include "AttendanceWidget.h"
#include "DutyWidget.h"
#include "LeaveWidget.h"
#include "SalaryWidget.h"
#include "UniformWidget.h"

#include <QApplication>
#include <QScreen>
#include <QFrame>
#include <QMessageBox>

MainWindow::MainWindow(const QString& username, const QString& role,
                       int userId, QWidget* parent)
    : QMainWindow(parent), m_username(username), m_role(role), m_userId(userId)
{
    buildUI();
    setWindowTitle("Security Guard Management System");
    resize(1280, 800);
    if (auto screen = QApplication::primaryScreen()) {
        auto geo = screen->availableGeometry();
        move((geo.width() - width()) / 2, (geo.height() - height()) / 2);
    }
    showDashboard();
}

void MainWindow::buildUI()
{
    auto* centralWidget = new QWidget;
    auto* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(createSidebar());

    m_stack = new QStackedWidget;

    m_dashboard  = new DashboardWidget;
    m_guards     = new GuardWidget;
    m_clients    = new ClientWidget;
    m_sites      = new SiteWidget;
    m_attendance = new AttendanceWidget(nullptr, m_userId);
    m_duty       = new DutyWidget;
    m_leave      = new LeaveWidget(nullptr, m_userId);
    m_salary     = new SalaryWidget;
    m_uniform    = new UniformWidget;

    m_stack->addWidget(m_dashboard);   // 0
    m_stack->addWidget(m_guards);      // 1
    m_stack->addWidget(m_clients);     // 2
    m_stack->addWidget(m_sites);       // 3
    m_stack->addWidget(m_attendance);  // 4
    m_stack->addWidget(m_duty);        // 5
    m_stack->addWidget(m_leave);       // 6
    m_stack->addWidget(m_salary);      // 7
    m_stack->addWidget(m_uniform);     // 8

    auto ph = [](const QString& title) {
        auto* w = new QWidget;
        auto* l = new QVBoxLayout(w);
        l->addStretch();
        auto* label = new QLabel(title + "\n\nComing Soon");
        label->setObjectName("PageTitle");
        label->setAlignment(Qt::AlignCenter);
        l->addWidget(label);
        l->addStretch();
        return w;
    };

    m_stack->addWidget(ph("Equipment"));        // 9
    m_stack->addWidget(ph("Visitor Register"));  // 10
    m_stack->addWidget(ph("Vehicle Register"));  // 11
    m_stack->addWidget(ph("Incidents"));         // 12
    m_stack->addWidget(ph("Training"));          // 13
    m_stack->addWidget(ph("Documents"));         // 14
    m_stack->addWidget(ph("Reports"));           // 15
    m_stack->addWidget(ph("Search"));            // 16
    m_stack->addWidget(ph("Backup"));            // 17
    m_stack->addWidget(ph("Settings"));          // 18

    mainLayout->addWidget(m_stack, 1);
    setCentralWidget(centralWidget);

    connect(m_dashboard, &DashboardWidget::navigateToGuard, this, &MainWindow::showGuards);
    connect(m_dashboard, &DashboardWidget::navigateToAttendance, this, &MainWindow::showAttendance);
    connect(m_dashboard, &DashboardWidget::navigateToSalary, this, &MainWindow::showSalary);
}

QWidget* MainWindow::createSidebar()
{
    auto* sidebar = new QWidget;
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(240);

    auto* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(2);

    auto* logo = new QLabel("SGMS");
    logo->setObjectName("SidebarTitle");
    sidebarLayout->addWidget(logo);

    auto* subtitle = new QLabel("Security Guard Manager");
    subtitle->setObjectName("SidebarSubtitle");
    sidebarLayout->addWidget(subtitle);

    auto addDivider = [&]() {
        auto* div = new QFrame;
        div->setObjectName("SidebarDivider");
        div->setFrameShape(QFrame::HLine);
        sidebarLayout->addWidget(div);
    };

    auto addSection = [&](const QString& text) {
        addDivider();
        auto* label = new QLabel(text);
        label->setObjectName("SectionTitle");
        label->setStyleSheet("padding: 8px 16px 4px 16px; color: #3D4654;");
        sidebarLayout->addWidget(label);
    };

    auto makeBtn = [&](const QString& text, int index) -> QPushButton* {
        auto* btn = new QPushButton(text);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        sidebarLayout->addWidget(btn);
        connect(btn, &QPushButton::clicked, this, [this, btn, index]() {
            m_stack->setCurrentIndex(index);
            setActiveNavButton(btn);
        });
        return btn;
    };

    sidebarLayout->addSpacing(8);

    addSection("MAIN");
    m_btnDashboard = makeBtn("  Dashboard",     0);
    m_btnGuards    = makeBtn("  Guards",        1);
    m_btnClients   = makeBtn("  Clients",       2);
    m_btnSites     = makeBtn("  Sites",         3);

    addSection("OPERATIONS");
    m_btnAttendance = makeBtn("  Attendance",      4);
    m_btnDuty       = makeBtn("  Duty Allocation", 5);
    m_btnLeave      = makeBtn("  Leave",           6);
    m_btnSalary     = makeBtn("  Salary",          7);

    addSection("RESOURCES");
    m_btnUniform   = makeBtn("  Uniform",    8);
    m_btnEquipment = makeBtn("  Equipment",  9);
    m_btnVisitors  = makeBtn("  Visitors",   10);
    m_btnVehicles  = makeBtn("  Vehicles",   11);

    addSection("ADMIN");
    m_btnIncidents = makeBtn("  Incidents",  12);
    m_btnTraining  = makeBtn("  Training",   13);
    m_btnDocuments = makeBtn("  Documents",  14);
    m_btnReports   = makeBtn("  Reports",    15);
    m_btnSearch    = makeBtn("  Search",     16);
    m_btnBackup    = makeBtn("  Backup",     17);
    m_btnSettings  = makeBtn("  Settings",   18);

    sidebarLayout->addStretch();
    addDivider();

    auto* userLabel = new QLabel(QString("  %1 (%2)").arg(m_username, m_role));
    userLabel->setStyleSheet("padding: 8px 16px; color: #6B7585; font-size: 12px;");
    sidebarLayout->addWidget(userLabel);

    auto* logoutBtn = new QPushButton("  Sign Out");
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: #E85454; "
        "padding: 10px 16px; text-align: left; font-weight: 600; }"
        "QPushButton:hover { background-color: #2A1A1A; }");
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::handleLogout);
    sidebarLayout->addWidget(logoutBtn);
    sidebarLayout->addSpacing(8);

    return sidebar;
}

void MainWindow::setActiveNavButton(QPushButton* activeBtn)
{
    QList<QPushButton*> allBtns = {
        m_btnDashboard, m_btnGuards, m_btnClients, m_btnSites,
        m_btnAttendance, m_btnDuty, m_btnLeave, m_btnSalary,
        m_btnUniform, m_btnEquipment, m_btnVisitors, m_btnVehicles,
        m_btnIncidents, m_btnTraining, m_btnDocuments, m_btnReports,
        m_btnSearch, m_btnBackup, m_btnSettings
    };
    for (auto* btn : allBtns) { if (btn) btn->setChecked(btn == activeBtn); }
}

void MainWindow::showDashboard() { m_stack->setCurrentIndex(0); setActiveNavButton(m_btnDashboard); m_dashboard->refresh(); }
void MainWindow::showGuards() { m_stack->setCurrentIndex(1); setActiveNavButton(m_btnGuards); m_guards->refresh(); }
void MainWindow::showClients() { m_stack->setCurrentIndex(2); setActiveNavButton(m_btnClients); m_clients->refresh(); }
void MainWindow::showSites() { m_stack->setCurrentIndex(3); setActiveNavButton(m_btnSites); m_sites->refresh(); }
void MainWindow::showAttendance() { m_stack->setCurrentIndex(4); setActiveNavButton(m_btnAttendance); m_attendance->refresh(); }
void MainWindow::showDuty() { m_stack->setCurrentIndex(5); setActiveNavButton(m_btnDuty); m_duty->refresh(); }
void MainWindow::showLeave() { m_stack->setCurrentIndex(6); setActiveNavButton(m_btnLeave); m_leave->refresh(); }
void MainWindow::showSalary() { m_stack->setCurrentIndex(7); setActiveNavButton(m_btnSalary); m_salary->refresh(); }
void MainWindow::showUniform() { m_stack->setCurrentIndex(8); setActiveNavButton(m_btnUniform); m_uniform->refresh(); }

void MainWindow::handleLogout()
{
    auto result = QMessageBox::question(this, "Sign Out", "Are you sure you want to sign out?",
                                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (result == QMessageBox::Yes) QApplication::quit();
}
