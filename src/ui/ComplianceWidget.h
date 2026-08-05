#pragma once
#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>

class ComplianceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ComplianceWidget(QWidget* parent = nullptr);
    void refresh();
private slots:
    void addFiling();
    void editFiling();
    void deleteFiling();
    void addLicense();
    void editLicense();
    void deleteLicense();
    void addMinWage();
    void editMinWage();
    void deleteMinWage();
    void loadDashboard();
    void loadFilings();
    void loadLicenses();
    void loadMinWages();
    void checkCompliance();
    void filterFilings();
    void filterLicenses();
private:
    void buildUI();
    QWidget* buildDashboardTab();
    QWidget* buildFilingsTab();
    QWidget* buildLicensesTab();
    QWidget* buildMinWagesTab();
    QWidget* buildChecklistTab();

    QTabWidget* m_tabs;
    // Dashboard
    QLabel* m_psaraLabel;
    QLabel* m_epfLabel;
    QLabel* m_esicLabel;
    QLabel* m_ptLabel;
    QLabel* m_bonusLabel;
    QLabel* m_gratuityLabel;
    QLabel* m_poshLabel;
    QLabel* m_wageLabel;
    QLabel* m_alertsLabel;
    // Filings
    QTableWidget* m_filingTable;
    QLineEdit* m_filingSearch;
    QComboBox* m_filingAreaFilter;
    QComboBox* m_filingStatusFilter;
    QLabel* m_filingSummary;
    // Licenses
    QTableWidget* m_licenseTable;
    QLineEdit* m_licenseSearch;
    QComboBox* m_licenseTypeFilter;
    QLabel* m_licenseSummary;
    // Min Wages
    QTableWidget* m_wageTable;
    QLabel* m_wageSummary;
    // Checklist
    QTableWidget* m_checklistTable;
};
