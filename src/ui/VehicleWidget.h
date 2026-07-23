#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class VehicleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VehicleWidget(QWidget* parent = nullptr);
    void refresh();

private slots:
    void addVehicle();
    void editVehicle();
    void markExit();
    void deleteVehicle();
    void loadVehicles();
    void filterVehicles(const QString& text);
    void exportCSV();

private:
    void buildUI();

    QTableWidget* m_table;
    QLineEdit*    m_searchEdit;
    QComboBox*    m_siteFilter;
    QComboBox*    m_statusFilter;
    QPushButton*  m_addBtn;
    QPushButton*  m_editBtn;
    QPushButton*  m_exitBtn;
    QPushButton*  m_deleteBtn;
    QPushButton*  m_exportBtn;
    QLabel*       m_countLabel;
    QLabel*       m_summaryLabel;
};
