#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class FineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FineWidget(QWidget* parent = nullptr);
    void refresh();
private slots:
    void addFine();
    void editFine();
    void approveFine();
    void deleteFine();
    void loadFines();
    void filterFines(const QString& text);
    void exportCSV();
    void showGuardSummary();
private:
    void buildUI();
    QTableWidget* m_table;
    QLineEdit* m_searchEdit;
    QComboBox* m_statusFilter;
    QComboBox* m_typeFilter;
    QLabel* m_countLabel;
    QLabel* m_summaryLabel;
};
