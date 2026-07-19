#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QLabel>
#include <QVariantMap>

class ClientDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ClientDialog(QWidget* parent = nullptr, int clientId = -1);

    QVariantMap clientData() const;

private slots:
    void saveClient();

private:
    void buildUI();
    void loadClientData();
    bool validate();

    int m_clientId;
    bool m_editMode;

    QLineEdit*       m_clientCode;
    QLineEdit*       m_companyName;
    QLineEdit*       m_contactPerson;
    QLineEdit*       m_mobile;
    QLineEdit*       m_email;
    QLineEdit*       m_address;
    QLineEdit*       m_city;
    QLineEdit*       m_gstNumber;
    QDoubleSpinBox*  m_billingRate;
    QDateEdit*       m_contractStart;
    QDateEdit*       m_contractEnd;
    QLineEdit*       m_agreementPath;
    QTextEdit*       m_invoiceTerms;
    QComboBox*       m_status;
    QTextEdit*       m_notes;
    QLabel*          m_errorLabel;
};
