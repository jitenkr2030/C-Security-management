#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QLabel>

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget* parent = nullptr);

    void refresh();

signals:
    void navigateToGuard();
    void navigateToAttendance();
    void navigateToSalary();
    void navigateToReports();
    void navigateToBackup();

private:
    void buildUI();
    QWidget* createStatCard(const QString& value, const QString& label);

    QLabel* m_totalGuardsVal;
    QLabel* m_presentVal;
    QLabel* m_absentVal;
    QLabel* m_clientsVal;
    QLabel* m_sitesVal;
    QLabel* m_salaryVal;
    QLabel* m_incidentsVal;
};
