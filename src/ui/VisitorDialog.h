#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QLabel>
#include <QTimeEdit>

class VisitorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VisitorDialog(QWidget* parent = nullptr, int visitorId = -1);

private slots:
    void saveVisitor();

private:
    void buildUI();
    void loadVisitorData();
    void loadSiteCombo();

    int  m_visitorId;
    bool m_editMode;

    QComboBox*  m_siteCombo;
    QLineEdit*  m_nameEdit;
    QLineEdit*  m_mobileEdit;
    QComboBox*  m_idProofCombo;
    QLineEdit*  m_idNumberEdit;
    QLineEdit*  m_purposeEdit;
    QLineEdit*  m_whomToMeet;
    QTimeEdit*  m_entryTime;
    QLineEdit*  m_vehicleNo;
    QTextEdit*  m_notes;
    QLabel*     m_errorLabel;
};
