#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QDateTimeEdit>
#include <QLabel>

class IncidentDialog : public QDialog
{
    Q_OBJECT
public:
    explicit IncidentDialog(QWidget* parent = nullptr, int incidentId = -1);
private slots:
    void saveIncident();
    void browsePhoto();
    void browseDocument();
private:
    void buildUI();
    void loadIncidentData();
    void loadSiteCombo();
    void loadGuardCombo();
    void updatePhotoPreview();
    int m_incidentId;
    bool m_editMode;
    QLineEdit* m_codeEdit;
    QComboBox* m_typeCombo;
    QComboBox* m_severityCombo;
    QComboBox* m_siteCombo;
    QComboBox* m_guardCombo;
    QDateTimeEdit* m_dateTime;
    QTextEdit* m_description;
    QTextEdit* m_actionTaken;
    QLineEdit* m_reportedBy;
    QLineEdit* m_witnessName;
    QLineEdit* m_witnessContact;
    QComboBox* m_statusCombo;
    QTextEdit* m_resolution;
    QLineEdit* m_photoPath;
    QLabel* m_photoPreview;
    QLineEdit* m_docPath;
    QLabel* m_errorLabel;
};
