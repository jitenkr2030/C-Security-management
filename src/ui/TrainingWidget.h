#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class TrainingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TrainingWidget(QWidget* parent = nullptr);
    void refresh();
private slots:
    void addTraining();
    void editTraining();
    void deleteTraining();
    void loadTrainings();
    void filterTrainings(const QString& text);
    void exportCSV();
private:
    void buildUI();
    QTableWidget* m_table;
    QLineEdit* m_searchEdit;
    QComboBox* m_typeFilter;
    QLabel* m_countLabel;
    QLabel* m_summaryLabel;
};
