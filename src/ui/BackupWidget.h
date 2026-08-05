#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>

class BackupWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BackupWidget(QWidget* parent = nullptr);
    void refresh();
private slots:
    void createBackup();
    void restoreBackup();
    void loadBackupLog();
private:
    void buildUI();
    QTableWidget* m_table;
    QLabel* m_statusLabel;
    QLabel* m_dbSizeLabel;
};
