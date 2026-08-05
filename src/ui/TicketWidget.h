#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class TicketWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TicketWidget(QWidget* parent = nullptr);
    void refresh();
private slots:
    void addTicket();
    void editTicket();
    void resolveTicket();
    void deleteTicket();
    void loadTickets();
    void filterTickets(const QString& text);
    void exportCSV();
private:
    void buildUI();
    QTableWidget* m_table;
    QLineEdit* m_searchEdit;
    QComboBox* m_statusFilter;
    QComboBox* m_priorityFilter;
    QComboBox* m_categoryFilter;
    QLabel* m_countLabel;
    QLabel* m_summaryLabel;
};
