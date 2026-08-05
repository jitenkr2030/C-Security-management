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
#include "EquipmentWidget.h"
#include "VisitorWidget.h"
#include "VehicleWidget.h"
#include "IncidentWidget.h"
#include "TrainingWidget.h"
#include "DocumentWidget.h"
#include "ComplaintWidget.h"
#include "FineWidget.h"
#include "AlertWidget.h"
#include "ReportsWidget.h"
#include "SearchWidget.h"
#include "BackupWidget.h"
#include "SettingsWidget.h"
#include <QApplication>
#include <QScreen>
#include <QFrame>
#include <QMessageBox>
#include <QScrollArea>

MainWindow::MainWindow(const QString& username, const QString& role, int userId, QWidget* parent)
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
    m_dashboard = new DashboardWidget;
    m_guards = new GuardWidget;
    m_clients = new ClientWidget;
    m_sites = new SiteWidget;
    m_attendance = new AttendanceWidget(nullptr, m_userId);
    m_duty = new DutyWidget;
    m_leave = new LeaveWidget(nullptr, m_userId);
    m_salary = new SalaryWidget;
    m_uniform = new UniformWidget;
    m_equipment = new EquipmentWidget;
    m_visitors = new VisitorWidget;
    m_vehicles = new VehicleWidget;
    m_incidents = new IncidentWidget;
    m_training = new TrainingWidget;
    m_documents = new DocumentWidget;
    m_complaints = new ComplaintWidget;
    m_fines = new FineWidget;
    m_alerts = new AlertWidget;
    m_reports = new ReportsWidget;
    m_search = new SearchWidget;
    m_backup = new BackupWidget;
    m_settings = new SettingsWidget;

    m_stack->addWidget(m_dashboard);   // 0
    m_stack->addWidget(m_guards);      // 1
    m_stack->addWidget(m_clients);     // 2
    m_stack->addWidget(m_sites);       // 3
    m_stack->addWidget(m_attendance);  // 4
    m_stack->addWidget(m_duty);        // 5
    m_stack->addWidget(m_leave);       // 6
    m_stack->addWidget(m_salary);      // 7
    m_stack->addWidget(m_uniform);     // 8
    m_stack->addWidget(m_equipment);   // 9
    m_stack->addWidget(m_visitors);    // 10
    m_stack->addWidget(m_vehicles);    // 11
    m_stack->addWidget(m_incidents);   // 12
    m_stack->addWidget(m_training);    // 13
    m_stack->addWidget(m_documents);   // 14
    m_stack->addWidget(m_complaints);  // 15
    m_stack->addWidget(m_fines);       // 16
    m_stack->addWidget(m_alerts);      // 17
    m_stack->addWidget(m_reports);     // 18
    m_stack->addWidget(m_search);      // 19
    m_stack->addWidget(m_backup);      // 20
    m_stack->addWidget(m_settings);    // 21

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
    sidebarLayout->setSpacing(0);

    // Fixed header
    auto* headerWidget = new QWidget;
    auto* headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 12, 0, 0);
    headerLayout->setSpacing(2);
    auto* logo = new QLabel("SGMS");
    logo->setObjectName("SidebarTitle");
    headerLayout->addWidget(logo);
    auto* sub = new QLabel("Security Guard Manager");
    sub->setObjectName("SidebarSubtitle");
    headerLayout->addWidget(sub);
    sidebarLayout->addWidget(headerWidget);

    // Scrollable nav
    auto* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; } QScrollArea > QWidget > QWidget { background: transparent; }");

    auto* navWidget = new QWidget;
    auto* navLayout = new QVBoxLayout(navWidget);
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->setSpacing(2);

    auto addDivider = [&]() {
        auto* div = new QFrame;
        div->setObjectName("SidebarDivider");
        div->setFrameShape(QFrame::HLine);
        navLayout->addWidget(div);
    };
    auto addSection = [&](const QString& text) {
        addDivider();
        auto* label = new QLabel(text);
        label->setObjectName("SectionTitle");
        label->setStyleSheet("padding: 8px 16px 4px 16px; color: #3D4654;");
        navLayout->addWidget(label);
    };
    auto makeBtn = [&](const QString& text, int index) -> QPushButton* {
        auto* btn = new QPushButton(text);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        navLayout->addWidget(btn);
        connect(btn, &QPushButton::clicked, this, [this, btn, index]() {
            m_stack->setCurrentIndex(index);
            setActiveNavButton(btn);
        });
        return btn;
    };

    navLayout->addSpacing(8);
    addSection("MAIN");
    m_btnDashboard = makeBtn("  Dashboard", 0);
    m_btnGuards = makeBtn("  Guards", 1);
    m_btnClients = makeBtn("  Clients", 2);
    m_btnSites = makeBtn("  Sites", 3);

    addSection("OPERATIONS");
    m_btnAttendance = makeBtn("  Attendance", 4);
    m_btnDuty = makeBtn("  Duty Allocation", 5);
    m_btnLeave = makeBtn("  Leave", 6);
    m_btnSalary = makeBtn("  Salary", 7);

    addSection("RESOURCES");
    m_btnUniform = makeBtn("  Uniform", 8);
    m_btnEquipment = makeBtn("  Equipment", 9);
    m_btnVisitors = makeBtn("  Visitors", 10);
    m_btnVehicles = makeBtn("  Vehicles", 11);

    addSection("ADMIN");
    m_btnIncidents = makeBtn("  Incidents", 12);
    m_btnTraining = makeBtn("  Training", 13);
    m_btnDocuments = makeBtn("  Documents", 14);
    m_btnComplaints = makeBtn("  Complaints", 15);
    m_btnFines = makeBtn("  Fines & Deductions", 16);
    m_btnAlerts = makeBtn("  Alerts & Notifications", 17);

    addSection("TOOLS");
    m_btnReports = makeBtn("  Reports", 18);
    m_btnSearch = makeBtn("  Search", 19);
    m_btnBackup = makeBtn("  Backup", 20);
    m_btnSettings = makeBtn("  Settings", 21);

    navLayout->addStretch();

    scrollArea->setWidget(navWidget);
    sidebarLayout->addWidget(scrollArea, 1);

    // Fixed footer
    auto* footerWidget = new QWidget;
    auto* footerLayout = new QVBoxLayout(footerWidget);
    footerLayout->setContentsMargins(0, 0, 0, 0);
    footerLayout->setSpacing(0);

    auto* div = new QFrame;
    div->setObjectName("SidebarDivider");
    div->setFrameShape(QFrame::HLine);
    footerLayout->addWidget(div);

    auto* userLabel = new QLabel(QString("  %1 (%2)").arg(m_username, m_role));
    userLabel->setStyleSheet("padding: 8px 16px; color: #6B7585; font-size: 12px;");
    footerLayout->addWidget(userLabel);

    auto* logoutBtn = new QPushButton("  Sign Out");
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: #E85454; "
        "padding: 10px 16px; text-align: left; font-weight: 600; } "
        "QPushButton:hover { background-color: #2A1A1A; }");
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::handleLogout);
    footerLayout->addWidget(logoutBtn);
    footerLayout->addSpacing(8);

    sidebarLayout->addWidget(footerWidget);

    return sidebar;
}

void MainWindow::setActiveNavButton(QPushButton* activeBtn)
{
    QList<QPushButton*> allBtns = {
        m_btnDashboard, m_btnGuards, m_btnClients, m_btnSites,
        m_btnAttendance, m_btnDuty, m_btnLeave, m_btnSalary,
        m_btnUniform, m_btnEquipment, m_btnVisitors, m_btnVehicles,
        m_btnIncidents, m_btnTraining, m_btnDocuments,
        m_btnComplaints, m_btnFines, m_btnAlerts,
        m_btnReports, m_btnSearch, m_btnBackup, m_btnSettings
    };
    for (auto* btn : allBtns) { if (btn) btn->setChecked(btn == activeBtn); }
}

void MainWindow::showDashboard()   { m_stack->setCurrentIndex(0);  setActiveNavButton(m_btnDashboard); m_dashboard->refresh(); }
void MainWindow::showGuards()      { m_stack->setCurrentIndex(1);  setActiveNavButton(m_btnGuards); m_guards->refresh(); }
void MainWindow::showClients()     { m_stack->setCurrentIndex(2);  setActiveNavButton(m_btnClients); m_clients->refresh(); }
void MainWindow::showSites()       { m_stack->setCurrentIndex(3);  setActiveNavButton(m_btnSites); m_sites->refresh(); }
void MainWindow::showAttendance()  { m_stack->setCurrentIndex(4);  setActiveNavButton(m_btnAttendance); m_attendance->refresh(); }
void MainWindow::showDuty()        { m_stack->setCurrentIndex(5);  setActiveNavButton(m_btnDuty); m_duty->refresh(); }
void MainWindow::showLeave()       { m_stack->setCurrentIndex(6);  setActiveNavButton(m_btnLeave); m_leave->refresh(); }
void MainWindow::showSalary()      { m_stack->setCurrentIndex(7);  setActiveNavButton(m_btnSalary); m_salary->refresh(); }
void MainWindow::showUniform()     { m_stack->setCurrentIndex(8);  setActiveNavButton(m_btnUniform); m_uniform->refresh(); }
void MainWindow::showEquipment()   { m_stack->setCurrentIndex(9);  setActiveNavButton(m_btnEquipment); m_equipment->refresh(); }
void MainWindow::showVisitors()    { m_stack->setCurrentIndex(10); setActiveNavButton(m_btnVisitors); m_visitors->refresh(); }
void MainWindow::showVehicles()    { m_stack->setCurrentIndex(11); setActiveNavButton(m_btnVehicles); m_vehicles->refresh(); }
void MainWindow::showIncidents()   { m_stack->setCurrentIndex(12); setActiveNavButton(m_btnIncidents); m_incidents->refresh(); }
void MainWindow::showTraining()    { m_stack->setCurrentIndex(13); setActiveNavButton(m_btnTraining); m_training->refresh(); }
void MainWindow::showDocuments()   { m_stack->setCurrentIndex(14); setActiveNavButton(m_btnDocuments); m_documents->refresh(); }
void MainWindow::showComplaints()  { m_stack->setCurrentIndex(15); setActiveNavButton(m_btnComplaints); m_complaints->refresh(); }
void MainWindow::showFines()       { m_stack->setCurrentIndex(16); setActiveNavButton(m_btnFines); m_fines->refresh(); }
void MainWindow::showAlerts()      { m_stack->setCurrentIndex(17); setActiveNavButton(m_btnAlerts); m_alerts->refresh(); }
void MainWindow::showReports()     { m_stack->setCurrentIndex(18); setActiveNavButton(m_btnReports); m_reports->refresh(); }
void MainWindow::showSearch()      { m_stack->setCurrentIndex(19); setActiveNavButton(m_btnSearch); m_search->refresh(); }
void MainWindow::showBackup()      { m_stack->setCurrentIndex(20); setActiveNavButton(m_btnBackup); m_backup->refresh(); }
void MainWindow::showSettings()    { m_stack->setCurrentIndex(21); setActiveNavButton(m_btnSettings); m_settings->refresh(); }

void MainWindow::handleLogout()
{
    auto result = QMessageBox::question(this, "Sign Out", "Are you sure you want to sign out?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (result == QMessageBox::Yes) QApplication::quit();
}
