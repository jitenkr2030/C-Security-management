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
#include "PhotoWidget.h"
#include "InvoiceWidget.h"
#include "TicketWidget.h"
#include "AuditLogWidget.h"
#include "RoleManagerWidget.h"
#include "ComplianceWidget.h"
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
    m_photos = new PhotoWidget;              // 20
    m_invoices = new InvoiceWidget;          // 21
    m_tickets = new TicketWidget;            // 22
    m_auditLog = new AuditLogWidget;         // 23
    m_roleManager = new RoleManagerWidget;   // 24
    m_compliance = new ComplianceWidget;     // 25
    m_reports = new ReportsWidget;           // 26
    m_search = new SearchWidget;             // 27
    m_backup = new BackupWidget;             // 28
    m_settings = new SettingsWidget;         // 29

    QList<QWidget*> widgets = {
        m_dashboard, m_guards, m_clients, m_sites, m_attendance, m_duty,
        m_leave, m_salary, m_uniform, m_equipment, m_visitors, m_vehicles,
        m_incidents, m_training, m_documents, m_complaints, m_fines, m_alerts,
        m_payroll, m_announcements, m_photos, m_invoices, m_tickets, m_auditLog,
        m_roleManager, m_compliance, m_reports, m_search, m_backup, m_settings
    };
    for (auto* w : widgets) m_stack->addWidget(w);

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
    m_btnPhotos = makeBtn("  Photo Gallery", idx++); m_btnInvoices = makeBtn("  Client Billing", idx++);
    m_btnTickets = makeBtn("  Helpdesk", idx++); m_btnAuditLog = makeBtn("  Audit Log", idx++);
    m_btnRoleManager = makeBtn("  Roles & Access", idx++); m_btnCompliance = makeBtn("  Compliance", idx++);

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
        m_btnPhotos, m_btnInvoices, m_btnTickets, m_btnAuditLog,
        m_btnRoleManager, m_btnCompliance,
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
void MainWindow::showPhotos()        { m_stack->setCurrentIndex(20); setActiveNavButton(m_btnPhotos); m_photos->refresh(); }
void MainWindow::showInvoices()      { m_stack->setCurrentIndex(21); setActiveNavButton(m_btnInvoices); m_invoices->refresh(); }
void MainWindow::showTickets()       { m_stack->setCurrentIndex(22); setActiveNavButton(m_btnTickets); m_tickets->refresh(); }
void MainWindow::showAuditLog()      { m_stack->setCurrentIndex(23); setActiveNavButton(m_btnAuditLog); m_auditLog->refresh(); }
void MainWindow::showRoleManager()   { m_stack->setCurrentIndex(24); setActiveNavButton(m_btnRoleManager); m_roleManager->refresh(); }
void MainWindow::showCompliance()    { m_stack->setCurrentIndex(25); setActiveNavButton(m_btnCompliance); m_compliance->refresh(); }
void MainWindow::showReports()       { m_stack->setCurrentIndex(26); setActiveNavButton(m_btnReports); m_reports->refresh(); }
void MainWindow::showSearch()        { m_stack->setCurrentIndex(27); setActiveNavButton(m_btnSearch); m_search->refresh(); }
void MainWindow::showBackup()        { m_stack->setCurrentIndex(28); setActiveNavButton(m_btnBackup); m_backup->refresh(); }
void MainWindow::showSettings()      { m_stack->setCurrentIndex(29); setActiveNavButton(m_btnSettings); m_settings->refresh(); }

void MainWindow::handleLogout()
{
    auto result = QMessageBox::question(this, "Sign Out", "Are you sure you want to sign out?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (result == QMessageBox::Yes) QApplication::quit();
}
