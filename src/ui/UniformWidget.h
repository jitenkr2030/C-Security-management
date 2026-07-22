#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class UniformWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UniformWidget(QWidget* parent = nullptr);
    void refresh();

private slots:
    void addItem();
    void editItem();
    void deleteItem();
    void issueUniform();
    void returnUniform();
    void loadInventory();
    void loadIssuance();
    void filterIssuance(const QString& text);
    void exportCSV();

private:
    void buildUI();
    QWidget* buildInventoryTab();
    QWidget* buildIssuanceTab();

    // Inventory tab
    QTableWidget* m_invTable;
    QLabel*       m_invSummary;

    // Issuance tab
    QTableWidget* m_issTable;
    QLineEdit*    m_searchEdit;
    QComboBox*    m_statusFilter;
    QComboBox*    m_itemFilter;
    QPushButton*  m_issueBtn;
    QPushButton*  m_returnBtn;
    QPushButton*  m_exportBtn;
    QLabel*       m_issSummary;
};
