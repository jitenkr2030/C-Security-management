#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class RoleManagerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RoleManagerWidget(QWidget* parent = nullptr);
    void refresh();
private slots:
    void loadPermissions();
    void savePermissions();
    void roleChanged(const QString& role);
private:
    void buildUI();
    QComboBox* m_roleCombo;
    QTableWidget* m_table;
    QLabel* m_infoLabel;
};
