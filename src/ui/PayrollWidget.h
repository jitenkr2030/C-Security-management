#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>

class PayrollWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PayrollWidget(QWidget* parent = nullptr);
    void refresh();
private slots:
    void generatePayroll();
    void markAllPaid();
    void exportCSV();
    void exportPayslips();
    void loadPayroll();
private:
    void buildUI();
    QTableWidget* m_table;
    QComboBox* m_monthCombo;
    QSpinBox* m_yearSpin;
    QLabel* m_countLabel;
    QLabel* m_summaryLabel;
    QPushButton* m_generateBtn;
};
