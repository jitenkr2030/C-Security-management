#pragma once

#include <QDialog>
#include <QComboBox>
#include <QDateEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QLabel>
#include <QVariantMap>

class DutyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DutyDialog(QWidget* parent = nullptr, int dutyId = -1);

private slots:
    void saveDuty();

private:
    void buildUI();
    void loadDutyData();
    void loadGuardCombo();
    void loadSiteCombo();
    bool validate();

    int  m_dutyId;
    bool m_editMode;

    QComboBox*  m_guardCombo;
    QComboBox*  m_siteCombo;
    QComboBox*  m_shiftCombo;
    QDateEdit*  m_startDate;
    QDateEdit*  m_endDate;
    QCheckBox*  m_permanentCheck;
    QComboBox*  m_dutyTypeCombo;
    QTextEdit*  m_notes;
    QLabel*     m_errorLabel;
};
