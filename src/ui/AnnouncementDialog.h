#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QDateTimeEdit>
#include <QLabel>

class AnnouncementDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AnnouncementDialog(QWidget* parent = nullptr, int announcementId = -1);
private slots:
    void saveAnnouncement();
private:
    void buildUI();
    void loadAnnouncementData();
    void loadSiteCombo();
    int m_announcementId;
    bool m_editMode;
    QLineEdit* m_titleEdit;
    QTextEdit* m_message;
    QComboBox* m_targetType;
    QComboBox* m_targetCombo;
    QComboBox* m_priorityCombo;
    QLineEdit* m_publishedBy;
    QDateTimeEdit* m_expiresAt;
    QComboBox* m_statusCombo;
    QLabel* m_errorLabel;
};
