#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class SiteWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SiteWidget(QWidget* parent = nullptr);

    void refresh();

private slots:
    void addSite();
    void editSite();
    void deleteSite();
    void viewSite();
    void filterSites(const QString& text);

private:
    void buildUI();
    void loadSites();
    int selectedSiteId() const;

    QTableWidget* m_table;
    QLineEdit*    m_searchEdit;
    QComboBox*    m_clientFilter;
    QComboBox*    m_statusFilter;
    QPushButton*  m_addBtn;
    QPushButton*  m_editBtn;
    QPushButton*  m_deleteBtn;
    QPushButton*  m_viewBtn;
    QLabel*       m_countLabel;
};
