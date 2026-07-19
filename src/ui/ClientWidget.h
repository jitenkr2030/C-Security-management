#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class ClientWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClientWidget(QWidget* parent = nullptr);

    void refresh();

private slots:
    void addClient();
    void editClient();
    void deleteClient();
    void viewClient();
    void filterClients(const QString& text);

private:
    void buildUI();
    void loadClients();
    int selectedClientId() const;

    QTableWidget* m_table;
    QLineEdit*    m_searchEdit;
    QComboBox*    m_statusFilter;
    QPushButton*  m_addBtn;
    QPushButton*  m_editBtn;
    QPushButton*  m_deleteBtn;
    QPushButton*  m_viewBtn;
    QLabel*       m_countLabel;
};
