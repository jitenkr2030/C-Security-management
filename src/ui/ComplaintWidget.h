#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class ComplaintWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ComplaintWidget(QWidget* parent = nullptr);
    void refresh();
private slots:
    void addComplaint();
    void editComplaint();
    void resolveComplaint();
    void deleteComplaint();
    void loadComplaints();
    void filterComplaints(const QString& text);
    void exportCSV();
private:
    void buildUI();
    QTableWidget* m_table;
    QLineEdit* m_searchEdit;
    QComboBox* m_statusFilter;
    QComboBox* m_typeFilter;
    QComboBox* m_severityFilter;
    QLabel* m_countLabel;
    QLabel* m_summaryLabel;
};
