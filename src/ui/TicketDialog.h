#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QLabel>

class TicketDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TicketDialog(QWidget* parent = nullptr, int ticketId = -1);
private slots:
    void saveTicket();
private:
    void buildUI();
    void loadTicketData();
    int m_ticketId;
    bool m_editMode;
    QLineEdit* m_codeEdit;
    QComboBox* m_categoryCombo;
    QComboBox* m_priorityCombo;
    QLineEdit* m_subjectEdit;
    QTextEdit* m_description;
    QLineEdit* m_raisedBy;
    QLineEdit* m_assignedTo;
    QComboBox* m_statusCombo;
    QTextEdit* m_resolution;
    QLabel* m_errorLabel;
};
