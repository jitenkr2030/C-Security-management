#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class LeaveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LeaveWidget(QWidget* parent = nullptr, int userId = 0);
    void refresh();

private slots:
    void applyLeave();
    void editLeave();
    void approveLeave();
    void rejectLeave();
    void deleteLeave();
    void loadLeaves();
    void loadBalanceTab();
    void filterLeaves(const QString& text);
    void exportCSV();

private:
    void buildUI();
    QWidget* buildRequestsTab();
    QWidget* buildBalanceTab();

    int m_userId;

    // Requests tab
    QTableWidget* m_table;
    QLineEdit*    m_searchEdit;
    QComboBox*    m_statusFilter;
    QComboBox*    m_typeFilter;
    QPushButton*  m_applyBtn;
    QPushButton*  m_editBtn;
    QPushButton*  m_approveBtn;
    QPushButton*  m_rejectBtn;
    QPushButton*  m_deleteBtn;
    QPushButton*  m_exportBtn;
    QLabel*       m_countLabel;

    // Balance tab
    QTableWidget* m_balanceTable;
};
