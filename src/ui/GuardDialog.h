#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QCheckBox>
#include <QLabel>
#include <QVariantMap>
class GuardDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GuardDialog(QWidget* parent = nullptr, int guardId = -1);
    QVariantMap guardData() const;
private slots:
    void saveGuard();
private:
    void buildUI();
    void loadGuardData();
    void loadClientCombo();
    void loadSiteCombo();
    bool validate();
    int m_guardId;
    bool m_editMode;
    QLineEdit* m_guardCode;
    QLineEdit* m_fullName;
    QLineEdit* m_fatherName;
    QDateEdit* m_dob;
    QComboBox* m_gender;
    QLineEdit* m_mobile1;
    QLineEdit* m_mobile2;
    QLineEdit* m_email;
    QLineEdit* m_address;
    QLineEdit* m_city;
    QLineEdit* m_state;
    QLineEdit* m_pincode;
    QLineEdit* m_aadhaar;
    QLineEdit* m_pan;
    QCheckBox* m_policeVerified;
    QDateEdit* m_joiningDate;
    QComboBox* m_status;
    QComboBox* m_clientCombo;
    QComboBox* m_siteCombo;
    QLineEdit* m_bankName;
    QLineEdit* m_bankAccount;
    QLineEdit* m_bankIfsc;
    QLineEdit* m_uan;
    QLineEdit* m_esic;
    QLineEdit* m_pf;
    QDoubleSpinBox* m_basicSalary;
    QDoubleSpinBox* m_hra;
    QDoubleSpinBox* m_conveyance;
    QDoubleSpinBox* m_medical;
    QDoubleSpinBox* m_special;
    QTextEdit* m_notes;
    QLabel* m_errorLabel;
};
