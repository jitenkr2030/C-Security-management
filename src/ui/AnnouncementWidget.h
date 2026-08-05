#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class AnnouncementWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AnnouncementWidget(QWidget* parent = nullptr);
    void refresh();
private slots:
    void addAnnouncement();
    void editAnnouncement();
    void deleteAnnouncement();
    void loadAnnouncements();
    void filterAnnouncements(const QString& text);
private:
    void buildUI();
    QTableWidget* m_table;
    QLineEdit* m_searchEdit;
    QComboBox* m_priorityFilter;
    QComboBox* m_statusFilter;
    QLabel* m_countLabel;
    QLabel* m_summaryLabel;
};
