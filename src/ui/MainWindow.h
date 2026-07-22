#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class DashboardWidget;
class GuardWidget;
class ClientWidget;
class SiteWidget;
class AttendanceWidget;
class DutyWidget;
class SalaryWidget;
class LeaveWidget;
class UniformWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString& username, const QString& role,
                        int userId = 0, QWidget* parent = nullptr);

private slots:
    void showDashboard();
    void showGuards();
    void showClients();
    void showSites();
    void showAttendance();
    void showDuty();
    void showLeave();
    void showSalary();
    void showUniform();
    void handleLogout();

private:
    void buildUI();
    QWidget* createSidebar();
    void setActiveNavButton(QPushButton* activeBtn);

    QStackedWidget*    m_stack;
    DashboardWidget*   m_dashboard;
    GuardWidget*       m_guards;
    ClientWidget*      m_clients;
    SiteWidget*        m_sites;
    AttendanceWidget*  m_attendance;
    DutyWidget*        m_duty;
    LeaveWidget*       m_leave;
    SalaryWidget*      m_salary;
    UniformWidget*     m_uniform;

    QPushButton* m_btnDashboard;
    QPushButton* m_btnGuards;
    QPushButton* m_btnClients;
    QPushButton* m_btnSites;
    QPushButton* m_btnAttendance;
    QPushButton* m_btnDuty;
    QPushButton* m_btnLeave;
    QPushButton* m_btnSalary;
    QPushButton* m_btnUniform;
    QPushButton* m_btnEquipment;
    QPushButton* m_btnVisitors;
    QPushButton* m_btnVehicles;
    QPushButton* m_btnIncidents;
    QPushButton* m_btnTraining;
    QPushButton* m_btnDocuments;
    QPushButton* m_btnReports;
    QPushButton* m_btnSearch;
    QPushButton* m_btnBackup;
    QPushButton* m_btnSettings;

    QString m_username;
    QString m_role;
    int     m_userId;
};
