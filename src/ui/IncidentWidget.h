#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class IncidentWidget : public QWidget
{
    Q_OBJECT
public:
    explicit IncidentWidget(QWidget* parent = nullptr);
    void refresh();
private slots:
    void addIncident();
    void editIncident();
    void resolveIncident();
    void deleteIncident();
    void loadIncidents();
    void filterIncidents(const QString& text);
    void exportCSV();
private:
    void buildUI();
    QTableWidget* m_table;
    QLineEdit* m_searchEdit;
    QComboBox* m_statusFilter;
    QComboBox* m_severityFilter;
    QLabel* m_countLabel;
    QLabel* m_summaryLabel;
};
