#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QDateEdit>

class AuditLogWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AuditLogWidget(QWidget* parent = nullptr);
    void refresh();
    static void log(const QString& action, const QString& module, int recordId = 0, const QString& details = "");
private slots:
    void loadLogs();
    void filterLogs(const QString& text);
    void exportCSV();
    void clearOldLogs();
private:
    void buildUI();
    QTableWidget* m_table;
    QLineEdit* m_searchEdit;
    QComboBox* m_moduleFilter;
    QComboBox* m_actionFilter;
    QDateEdit* m_fromDate;
    QLabel* m_countLabel;
    QLabel* m_summaryLabel;
};
