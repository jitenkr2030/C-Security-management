#pragma once

#include <QDialog>
#include <QComboBox>
#include <QDateEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QLabel>
#include <QVariantMap>

class LeaveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LeaveDialog(QWidget* parent = nullptr, int leaveId = -1, int userId = 0);

private slots:
    void saveLeave();

private:
    void buildUI();
    void loadLeaveData();
    void loadGuardCombo();
    void updateDayCount();

    int  m_leaveId;
    int  m_userId;
    bool m_editMode;

    QComboBox*  m_guardCombo;
    QComboBox*  m_leaveType;
    QDateEdit*  m_startDate;
    QDateEdit*  m_endDate;
    QSpinBox*   m_dayCount;
    QTextEdit*  m_reason;
    QLabel*     m_errorLabel;
    QLabel*     m_balanceLabel;
};
