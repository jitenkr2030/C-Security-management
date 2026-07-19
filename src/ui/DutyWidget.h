#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class DutyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DutyWidget(QWidget* parent = nullptr);

    void refresh();

private slots:
    void addDuty();
    void editDuty();
    void deleteDuty();
    void viewRoster();
    void transferGuard();
    void loadDuties();
    void filterDuties(const QString& text);

private:
    void buildUI();
    QWidget* buildRosterTab();
    int selectedDutyId() const;

    QTableWidget* m_table;
    QLineEdit*    m_searchEdit;
    QComboBox*    m_siteFilter;
    QComboBox*    m_shiftFilter;
    QComboBox*    m_typeFilter;
    QPushButton*  m_addBtn;
    QPushButton*  m_editBtn;
    QPushButton*  m_deleteBtn;
    QPushButton*  m_viewBtn;
    QPushButton*  m_transferBtn;
    QLabel*       m_countLabel;

    // Roster tab
    QDateEdit*    m_rosterDate;
    QComboBox*    m_rosterSite;
    QTableWidget* m_rosterTable;
};
