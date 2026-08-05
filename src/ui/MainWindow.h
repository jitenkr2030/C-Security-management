#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class DashboardWidget; class GuardWidget; class ClientWidget; class SiteWidget;
class AttendanceWidget; class DutyWidget; class SalaryWidget; class LeaveWidget;
class UniformWidget; class EquipmentWidget; class VisitorWidget; class VehicleWidget;
class IncidentWidget; class TrainingWidget; class DocumentWidget;
class ComplaintWidget; class FineWidget; class AlertWidget; class PayrollWidget;
class AnnouncementWidget; class PhotoWidget; class InvoiceWidget;
class TicketWidget; class AuditLogWidget; class RoleManagerWidget;
class ComplianceWidget;
class ReportsWidget; class SearchWidget; class BackupWidget; class SettingsWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const QString& username, const QString& role, int userId = 0, QWidget* parent = nullptr);
private slots:
    void showDashboard(); void showGuards(); void showClients(); void showSites();
    void showAttendance(); void showDuty(); void showLeave(); void showSalary();
    void showUniform(); void showEquipment(); void showVisitors(); void showVehicles();
    void showIncidents(); void showTraining(); void showDocuments();
    void showComplaints(); void showFines(); void showAlerts(); void showPayroll();
    void showAnnouncements(); void showPhotos(); void showInvoices();
    void showTickets(); void showAuditLog(); void showRoleManager(); void showCompliance();
    void showReports(); void showSearch(); void showBackup(); void showSettings();
    void handleLogout();
private:
    void buildUI();
    QWidget* createSidebar();
    void setActiveNavButton(QPushButton* activeBtn);

    QStackedWidget* m_stack;
    DashboardWidget* m_dashboard; GuardWidget* m_guards; ClientWidget* m_clients;
    SiteWidget* m_sites; AttendanceWidget* m_attendance; DutyWidget* m_duty;
    LeaveWidget* m_leave; SalaryWidget* m_salary; UniformWidget* m_uniform;
    EquipmentWidget* m_equipment; VisitorWidget* m_visitors; VehicleWidget* m_vehicles;
    IncidentWidget* m_incidents; TrainingWidget* m_training; DocumentWidget* m_documents;
    ComplaintWidget* m_complaints; FineWidget* m_fines; AlertWidget* m_alerts;
    PayrollWidget* m_payroll; AnnouncementWidget* m_announcements;
    PhotoWidget* m_photos; InvoiceWidget* m_invoices;
    TicketWidget* m_tickets; AuditLogWidget* m_auditLog;
    RoleManagerWidget* m_roleManager; ComplianceWidget* m_compliance;
    ReportsWidget* m_reports; SearchWidget* m_search; BackupWidget* m_backup;
    SettingsWidget* m_settings;

    QPushButton* m_btnDashboard; QPushButton* m_btnGuards; QPushButton* m_btnClients;
    QPushButton* m_btnSites; QPushButton* m_btnAttendance; QPushButton* m_btnDuty;
    QPushButton* m_btnLeave; QPushButton* m_btnSalary; QPushButton* m_btnUniform;
    QPushButton* m_btnEquipment; QPushButton* m_btnVisitors; QPushButton* m_btnVehicles;
    QPushButton* m_btnIncidents; QPushButton* m_btnTraining; QPushButton* m_btnDocuments;
    QPushButton* m_btnComplaints; QPushButton* m_btnFines; QPushButton* m_btnAlerts;
    QPushButton* m_btnPayroll; QPushButton* m_btnAnnouncements;
    QPushButton* m_btnPhotos; QPushButton* m_btnInvoices;
    QPushButton* m_btnTickets; QPushButton* m_btnAuditLog;
    QPushButton* m_btnRoleManager; QPushButton* m_btnCompliance;
    QPushButton* m_btnReports; QPushButton* m_btnSearch; QPushButton* m_btnBackup;
    QPushButton* m_btnSettings;

    QString m_username; QString m_role; int m_userId;
};
