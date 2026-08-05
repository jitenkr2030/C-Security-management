#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QDateEdit>
#include <QLabel>

class PhotoDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PhotoDialog(QWidget* parent = nullptr, int photoId = -1);
private slots:
    void savePhoto();
    void browsePhoto();
private:
    void buildUI();
    void loadPhotoData();
    void loadSiteCombo();
    void updatePreview();
    int m_photoId;
    bool m_editMode;
    QComboBox* m_categoryCombo;
    QLineEdit* m_titleEdit;
    QLineEdit* m_filePath;
    QLabel* m_preview;
    QComboBox* m_relatedType;
    QLineEdit* m_relatedId;
    QComboBox* m_siteCombo;
    QDateEdit* m_takenDate;
    QLineEdit* m_uploadedBy;
    QTextEdit* m_notes;
    QLabel* m_errorLabel;
};
