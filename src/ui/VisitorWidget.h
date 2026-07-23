#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QDateEdit>

class VisitorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisitorWidget(QWidget* parent = nullptr);
    void refresh();

private slots:
    void addVisitor();
    void editVisitor();
    void markExit();
    void deleteVisitor();
    void loadVisitors();
    void filterVisitors(const QString& text);
    void filterByDate();
    void exportCSV();

private:
    void buildUI();

    QTableWidget* m_table;
    QLineEdit*    m_searchEdit;
    QComboBox*    m_siteFilter;
    QComboBox*    m_statusFilter;
    QDateEdit*    m_dateFilter;
    QPushButton*  m_addBtn;
    QPushButton*  m_editBtn;
    QPushButton*  m_exitBtn;
    QPushButton*  m_deleteBtn;
    QPushButton*  m_exportBtn;
    QLabel*       m_countLabel;
    QLabel*       m_summaryLabel;
};
