#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>

class InvoiceDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InvoiceDialog(QWidget* parent = nullptr, int invoiceId = -1);
private slots:
    void saveInvoice();
    void calculateTotal();
    void autoFillFromSite();
private:
    void buildUI();
    void loadInvoiceData();
    void loadClientCombo();
    void loadSiteCombo();
    int m_invoiceId;
    bool m_editMode;
    QLineEdit* m_codeEdit;
    QComboBox* m_clientCombo;
    QComboBox* m_monthCombo;
    QSpinBox* m_yearSpin;
    QComboBox* m_siteCombo;
    QSpinBox* m_guardsDeployed;
    QSpinBox* m_workingDays;
    QDoubleSpinBox* m_perGuardRate;
    QDoubleSpinBox* m_totalGuardCharges;
    QDoubleSpinBox* m_equipmentCharges;
    QDoubleSpinBox* m_otherCharges;
    QDoubleSpinBox* m_subtotal;
    QDoubleSpinBox* m_gstRate;
    QDoubleSpinBox* m_gstAmount;
    QDoubleSpinBox* m_totalAmount;
    QComboBox* m_statusCombo;
    QDateEdit* m_invoiceDate;
    QDateEdit* m_dueDate;
    QTextEdit* m_notes;
    QLabel* m_errorLabel;
    QLabel* m_calcLabel;
};
