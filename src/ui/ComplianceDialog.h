#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QTabWidget>

class ComplianceDialog : public QDialog
{
    Q_OBJECT
public:
    enum Type { FilingType, LicenseType, MinWageType };
    explicit ComplianceDialog(QWidget* parent, Type type, int recordId = -1);
private slots:
    void saveRecord();
private:
    void buildFilingUI();
    void buildLicenseUI();
    void buildMinWageUI();
    void loadFilingData();
    void loadLicenseData();
    void loadMinWageData();

    Type m_type;
    int m_recordId;
    bool m_editMode;

    // Filing fields
    QComboBox* m_filingArea;
    QComboBox* m_filingType;
    QLineEdit* m_filingPeriod;
    QDateEdit* m_filingDueDate;
    QDateEdit* m_filingFiledDate;
    QDoubleSpinBox* m_filingAmount;
    QComboBox* m_filingStatus;
    QLineEdit* m_filingChallanPath;
    QTextEdit* m_filingNotes;

    // License fields
    QComboBox* m_licenseType;
    QLineEdit* m_licenseNumber;
    QLineEdit* m_issuingAuthority;
    QLineEdit* m_licenseState;
    QDateEdit* m_issueDate;
    QDateEdit* m_expiryDate;
    QComboBox* m_renewalStatus;
    QLineEdit* m_docPath;
    QTextEdit* m_licenseNotes;
    QComboBox* m_licenseStatus;

    // MinWage fields
    QLineEdit* m_wageState;
    QComboBox* m_wageZone;
    QComboBox* m_wageSkill;
    QDoubleSpinBox* m_basicWage;
    QDoubleSpinBox* m_vda;
    QDoubleSpinBox* m_totalWage;
    QDateEdit* m_effectiveFrom;
    QLineEdit* m_notificationNo;

    QLabel* m_errorLabel;
};
