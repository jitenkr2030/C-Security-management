#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class AlertWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AlertWidget(QWidget* parent = nullptr);
    void refresh();
private slots:
    void loadAlerts();
    void filterAlerts(const QString& text);
    void acknowledgeAlert();
    void resolveAlert();
    void exportCSV();
private:
    void buildUI();
    void scanLicenseAlerts();
    void scanComplianceAlerts();
    void scanSalaryAlerts();
    void scanLeaveAlerts();
    void scanComplaintAlerts();
    void scanIncidentAlerts();

    QTableWidget* m_table;
    QLineEdit* m_searchEdit;
    QComboBox* m_categoryFilter;
    QComboBox* m_priorityFilter;
    QLabel* m_countLabel;
    QLabel* m_summaryLabel;
};
