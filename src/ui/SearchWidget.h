#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>

class SearchWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SearchWidget(QWidget* parent = nullptr);
    void refresh();
private slots:
    void performSearch();
    void openResult();
private:
    void buildUI();
    void searchGuards(const QString& text);
    void searchClients(const QString& text);
    void searchSites(const QString& text);
    void searchVisitors(const QString& text);
    void searchVehicles(const QString& text);
    void searchIncidents(const QString& text);

    QLineEdit* m_searchEdit;
    QComboBox* m_entityCombo;
    QTableWidget* m_table;
    QLabel* m_resultLabel;
};
