#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QMap>

class SalaryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SalaryWidget(QWidget* parent = nullptr);
    void refresh();

private slots:
    void generateSalary();
    void markPaid();
    void loadSalaryData();
    void exportPayroll();
    void filterSalary(const QString& text);
    void viewSlip();

private:
    void buildUI();
    QWidget* buildPayrollTab();
    QWidget* buildSlipTab();
    void calculateSalaryForGuard(int guardId, int month, int year);
    void loadSettings();

    // Settings cache
    double m_pfRate;
    double m_esicRate;
    double m_ptDeduction;
    double m_overtimeRate;
    int    m_workingDays;

    // Payroll tab
    QComboBox*    m_monthCombo;
    QSpinBox*     m_yearSpin;
    QTableWidget* m_payrollTable;
    QLineEdit*    m_searchEdit;
    QComboBox*    m_statusFilter;
    QPushButton*  m_generateBtn;
    QPushButton*  m_payBtn;
    QPushButton*  m_exportBtn;
    QPushButton*  m_slipBtn;
    QLabel*       m_summaryLabel;

    // Slip tab
    QComboBox*    m_slipGuardCombo;
    QComboBox*    m_slipMonthCombo;
    QSpinBox*     m_slipYearSpin;
    QTableWidget* m_slipTable;
    QLabel*       m_slipSummary;
};
