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
#include "PayrollWidget.h"
#include "AnnouncementWidget.h"
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
    m_dashboard = new DashboardWidget;       // 0
    m_guards = new GuardWidget;              // 1
    m_clients = new ClientWidget;            // 2
    m_sites = new SiteWidget;                // 3
    m_attendance = new AttendanceWidget(nullptr, m_userId); // 4
    m_duty = new DutyWidget;                 // 5
    m_leave = new LeaveWidget(nullptr, m_userId); // 6
    m_salary = new SalaryWidget;             // 7
    m_uniform = new UniformWidget;           // 8
    m_equipment = new EquipmentWidget;       // 9
    m_visitors = new VisitorWidget;          // 10
    m_vehicles = new VehicleWidget;          // 11
    m_incidents = new IncidentWidget;        // 12
    m_training = new TrainingWidget;         // 13
    m_documents = new DocumentWidget;        // 14
    m_complaints = new ComplaintWidget;      // 15
    m_fines = new FineWidget;                // 16
    m_alerts = new AlertWidget;              // 17
    m_payroll = new PayrollWidget;           // 18
    m_announcements = new AnnouncementWidget;// 19
    m_reports = new ReportsWidget;           // 20
    m_search = new SearchWidget;             // 21
    m_backup = new BackupWidget;             // 22
    m_settings = new SettingsWidget;         // 23

    for (auto* w : {static_cast<QWidget*>(m_dashboard), static_cast<QWidget*>(m_guards),
         static_cast<QWidget*>(m_clients), static_cast<QWidget*>(m_sites),
         static_cast<QWidget*>(m_attendance), static_cast<QWidget*>(m_duty),
         static_cast<QWidget*>(m_leave), static_cast<QWidget*>(m_salary),
         static_cast<QWidget*>(m_uniform), static_cast<QWidget*>(m_equipment),
         static_cast<QWidget*>(m_visitors), static_cast<QWidget*>(m_vehicles),
         static_cast<QWidget*>(m_incidents), static_cast<QWidget*>(m_training),
         static_cast<QWidget*>(m_documents), static_cast<QWidget*>(m_complaints),
         static_cast<QWidget*>(m_fines), static_cast<QWidget*>(m_alerts),
         static_cast<QWidget*>(m_payroll), static_cast<QWidget*>(m_announcements),
         static_cast<QWidget*>(m_reports), static_cast<QWidget*>(m_search),
         static_cast<QWidget*>(m_backup), static_cast<QWidget*>(m_settings)})
        m_stack->addWidget(w);

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

    auto* headerWidget = new QWidget;
    auto* headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 12, 0, 0);
    headerLayout->setSpacing(2);
    auto* logo = new QLabel("SGMS"); logo->setObjectName("SidebarTitle"); headerLayout->addWidget(logo);
    auto* sub = new QLabel("Security Guard Manager"); sub->setObjectName("SidebarSubtitle"); headerLayout->addWidget(sub);
    sidebarLayout->addWidget(headerWidget);

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

    auto addDivider = [&]() { auto* d = new QFrame; d->setObjectName("SidebarDivider"); d->setFrameShape(QFrame::HLine); navLayout->addWidget(d); };
    auto addSection = [&](const QString& t) { addDivider(); auto* l = new QLabel(t); l->setObjectName("SectionTitle"); l->setStyleSheet("padding: 8px 16px 4px 16px; color: #3D4654;"); navLayout->addWidget(l); };
    auto makeBtn = [&](const QString& t, int i) -> QPushButton* {
        auto* b = new QPushButton(t); b->setCheckable(true); b->setCursor(Qt::PointingHandCursor); navLayout->addWidget(b);
        connect(b, &QPushButton::clicked, this, [this, b, i]() { m_stack->setCurrentIndex(i); setActiveNavButton(b); }); return b;
    };

    int idx = 0;
    navLayout->addSpacing(8);
    addSection("MAIN");
    m_btnDashboard = makeBtn("  Dashboard", idx++); m_btnGuards = makeBtn("  Guards", idx++);
    m_btnClients = makeBtn("  Clients", idx++); m_btnSites = makeBtn("  Sites", idx++);

    addSection("OPERATIONS");
    m_btnAttendance = makeBtn("  Attendance", idx++); m_btnDuty = makeBtn("  Duty Allocation", idx++);
    m_btnLeave = makeBtn("  Leave", idx++); m_btnSalary = makeBtn("  Salary", idx++);

    addSection("RESOURCES");
    m_btnUniform = makeBtn("  Uniform", idx++); m_btnEquipment = makeBtn("  Equipment", idx++);
    m_btnVisitors = makeBtn("  Visitors", idx++); m_btnVehicles = makeBtn("  Vehicles", idx++);

    addSection("ADMIN");
    m_btnIncidents = makeBtn("  Incidents", idx++); m_btnTraining = makeBtn("  Training", idx++);
    m_btnDocuments = makeBtn("  Documents", idx++); m_btnComplaints = makeBtn("  Complaints", idx++);
    m_btnFines = makeBtn("  Fines & Deductions", idx++); m_btnAlerts = makeBtn("  Alerts", idx++);
    m_btnPayroll = makeBtn("  Payroll", idx++); m_btnAnnouncements = makeBtn("  Announcements", idx++);

    addSection("TOOLS");
    m_btnReports = makeBtn("  Reports", idx++); m_btnSearch = makeBtn("  Search", idx++);
    m_btnBackup = makeBtn("  Backup", idx++); m_btnSettings = makeBtn("  Settings", idx++);

    navLayout->addStretch();
    scrollArea->setWidget(navWidget);
    sidebarLayout->addWidget(scrollArea, 1);

    auto* footerWidget = new QWidget;
    auto* footerLayout = new QVBoxLayout(footerWidget);
    footerLayout->setContentsMargins(0, 0, 0, 0);
    footerLayout->setSpacing(0);
    auto* div = new QFrame; div->setObjectName("SidebarDivider"); div->setFrameShape(QFrame::HLine); footerLayout->addWidget(div);
    auto* userLabel = new QLabel(QString("  %1 (%2)").arg(m_username, m_role));
    userLabel->setStyleSheet("padding: 8px 16px; color: #6B7585; font-size: 12px;");
    footerLayout->addWidget(userLabel);
    auto* logoutBtn = new QPushButton("  Sign Out"); logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setStyleSheet("QPushButton { background: transparent; border: none; color: #E85454; padding: 10px 16px; text-align: left; font-weight: 600; } QPushButton:hover { background-color: #2A1A1A; }");
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::handleLogout);
    footerLayout->addWidget(logoutBtn);
    footerLayout->addSpacing(8);
    sidebarLayout->addWidget(footerWidget);

    return sidebar;
}

void MainWindow::setActiveNavButton(QPushButton* activeBtn)
{
    QList<QPushButton*> allBtns = { m_btnDashboard, m_btnGuards, m_btnClients, m_btnSites,
        m_btnAttendance, m_btnDuty, m_btnLeave, m_btnSalary, m_btnUniform, m_btnEquipment,
        m_btnVisitors, m_btnVehicles, m_btnIncidents, m_btnTraining, m_btnDocuments,
        m_btnComplaints, m_btnFines, m_btnAlerts, m_btnPayroll, m_btnAnnouncements,
        m_btnReports, m_btnSearch, m_btnBackup, m_btnSettings };
    for (auto* btn : allBtns) { if (btn) btn->setChecked(btn == activeBtn); }
}

void MainWindow::showDashboard()     { m_stack->setCurrentIndex(0);  setActiveNavButton(m_btnDashboard); m_dashboard->refresh(); }
void MainWindow::showGuards()        { m_stack->setCurrentIndex(1);  setActiveNavButton(m_btnGuards); m_guards->refresh(); }
void MainWindow::showClients()       { m_stack->setCurrentIndex(2);  setActiveNavButton(m_btnClients); m_clients->refresh(); }
void MainWindow::showSites()         { m_stack->setCurrentIndex(3);  setActiveNavButton(m_btnSites); m_sites->refresh(); }
void MainWindow::showAttendance()    { m_stack->setCurrentIndex(4);  setActiveNavButton(m_btnAttendance); m_attendance->refresh(); }
void MainWindow::showDuty()          { m_stack->setCurrentIndex(5);  setActiveNavButton(m_btnDuty); m_duty->refresh(); }
void MainWindow::showLeave()         { m_stack->setCurrentIndex(6);  setActiveNavButton(m_btnLeave); m_leave->refresh(); }
void MainWindow::showSalary()        { m_stack->setCurrentIndex(7);  setActiveNavButton(m_btnSalary); m_salary->refresh(); }
void MainWindow::showUniform()       { m_stack->setCurrentIndex(8);  setActiveNavButton(m_btnUniform); m_uniform->refresh(); }
void MainWindow::showEquipment()     { m_stack->setCurrentIndex(9);  setActiveNavButton(m_btnEquipment); m_equipment->refresh(); }
void MainWindow::showVisitors()      { m_stack->setCurrentIndex(10); setActiveNavButton(m_btnVisitors); m_visitors->refresh(); }
void MainWindow::showVehicles()      { m_stack->setCurrentIndex(11); setActiveNavButton(m_btnVehicles); m_vehicles->refresh(); }
void MainWindow::showIncidents()     { m_stack->setCurrentIndex(12); setActiveNavButton(m_btnIncidents); m_incidents->refresh(); }
void MainWindow::showTraining()      { m_stack->setCurrentIndex(13); setActiveNavButton(m_btnTraining); m_training->refresh(); }
void MainWindow::showDocuments()     { m_stack->setCurrentIndex(14); setActiveNavButton(m_btnDocuments); m_documents->refresh(); }
void MainWindow::showComplaints()    { m_stack->setCurrentIndex(15); setActiveNavButton(m_btnComplaints); m_complaints->refresh(); }
void MainWindow::showFines()         { m_stack->setCurrentIndex(16); setActiveNavButton(m_btnFines); m_fines->refresh(); }
void MainWindow::showAlerts()        { m_stack->setCurrentIndex(17); setActiveNavButton(m_btnAlerts); m_alerts->refresh(); }
void MainWindow::showPayroll()       { m_stack->setCurrentIndex(18); setActiveNavButton(m_btnPayroll); m_payroll->refresh(); }
void MainWindow::showAnnouncements() { m_stack->setCurrentIndex(19); setActiveNavButton(m_btnAnnouncements); m_announcements->refresh(); }
void MainWindow::showReports()       { m_stack->setCurrentIndex(20); setActiveNavButton(m_btnReports); m_reports->refresh(); }
void MainWindow::showSearch()        { m_stack->setCurrentIndex(21); setActiveNavButton(m_btnSearch); m_search->refresh(); }
void MainWindow::showBackup()        { m_stack->setCurrentIndex(22); setActiveNavButton(m_btnBackup); m_backup->refresh(); }
void MainWindow::showSettings()      { m_stack->setCurrentIndex(23); setActiveNavButton(m_btnSettings); m_settings->refresh(); }

void MainWindow::handleLogout()
{
    auto result = QMessageBox::question(this, "Sign Out", "Are you sure you want to sign out?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (result == QMessageBox::Yes) QApplication::quit();
}
