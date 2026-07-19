#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class GuardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GuardWidget(QWidget* parent = nullptr);

    void refresh();

private slots:
    void addGuard();
    void editGuard();
    void deleteGuard();
    void viewProfile();
    void filterGuards(const QString& text);

private:
    void buildUI();
    void loadGuards();
    int selectedGuardId() const;

    QTableWidget* m_table;
    QLineEdit*    m_searchEdit;
    QComboBox*    m_statusFilter;
    QPushButton*  m_addBtn;
    QPushButton*  m_editBtn;
    QPushButton*  m_deleteBtn;
    QPushButton*  m_viewBtn;
    QLabel*       m_countLabel;
};
