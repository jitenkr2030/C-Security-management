#pragma once
#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QDateEdit>

class ReportsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ReportsWidget(QWidget* parent = nullptr);
    void refresh();
private slots:
    void generateReport();
    void exportReport();
private:
    void buildUI();
    void generateGuardReport();
    void generateAttendanceReport();
    void generateSalaryReport();
    void generateLeaveReport();
    void generateVisitorReport();
    void generateIncidentReport();
    void generateUniformReport();
    void generateEquipmentReport();

    QComboBox* m_reportType;
    QDateEdit* m_fromDate;
    QDateEdit* m_toDate;
    QComboBox* m_siteFilter;
    QTableWidget* m_table;
    QLabel* m_summaryLabel;
    QLabel* m_titleLabel;
    QPushButton* m_generateBtn;
    QPushButton* m_exportBtn;
};
