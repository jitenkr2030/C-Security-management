#pragma once
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>

class DocumentDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DocumentDialog(QWidget* parent = nullptr, int documentId = -1);
private slots:
    void saveDocument();
    void browseFile();
private:
    void buildUI();
    void loadDocumentData();
    void loadGuardCombo();
    void loadClientCombo();
    int m_documentId;
    bool m_editMode;
    QComboBox* m_guardCombo;
    QComboBox* m_clientCombo;
    QComboBox* m_typeCombo;
    QLineEdit* m_fileName;
    QLineEdit* m_filePath;
    QLabel* m_fileInfo;
    QTextEdit* m_desc;
    QLabel* m_errorLabel;
};
