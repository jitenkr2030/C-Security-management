#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QDateEdit>
#include <QLabel>

class ComplaintDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ComplaintDialog(QWidget* parent = nullptr, int complaintId = -1);
private slots:
    void saveComplaint();
private:
    void buildUI();
    void loadComplaintData();
    void loadClientCombo();
    void loadSiteCombo();
    void loadGuardCombo();
    int m_complaintId;
    bool m_editMode;
    QLineEdit* m_codeEdit;
    QComboBox* m_typeCombo;
    QComboBox* m_categoryCombo;
    QComboBox* m_sourceCombo;
    QComboBox* m_clientCombo;
    QComboBox* m_siteCombo;
    QComboBox* m_guardCombo;
    QLineEdit* m_complainantName;
    QLineEdit* m_complainantContact;
    QLineEdit* m_subjectEdit;
    QTextEdit* m_description;
    QComboBox* m_severityCombo;
    QComboBox* m_statusCombo;
    QLineEdit* m_assignedTo;
    QTextEdit* m_resolution;
    QLabel* m_errorLabel;
};
