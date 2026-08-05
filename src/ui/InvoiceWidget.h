#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class InvoiceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit InvoiceWidget(QWidget* parent = nullptr);
    void refresh();
private slots:
    void addInvoice();
    void editInvoice();
    void markPaid();
    void deleteInvoice();
    void loadInvoices();
    void filterInvoices(const QString& text);
    void exportCSV();
private:
    void buildUI();
    QTableWidget* m_table;
    QLineEdit* m_searchEdit;
    QComboBox* m_statusFilter;
    QComboBox* m_clientFilter;
    QLabel* m_countLabel;
    QLabel* m_summaryLabel;
};
