#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QLabel>
#include <QVariantMap>

class SiteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SiteDialog(QWidget* parent = nullptr, int siteId = -1);

    QVariantMap siteData() const;

private slots:
    void saveSite();

private:
    void buildUI();
    void loadSiteData();
    void loadClients();
    void loadSupervisors();
    bool validate();

    int m_siteId;
    bool m_editMode;

    QLineEdit*    m_siteCode;
    QLineEdit*    m_siteName;
    QLineEdit*    m_address;
    QLineEdit*    m_city;
    QComboBox*    m_clientCombo;
    QLineEdit*    m_shiftMorning;
    QLineEdit*    m_shiftAfternoon;
    QLineEdit*    m_shiftNight;
    QComboBox*    m_supervisorCombo;
    QSpinBox*     m_guardsRequired;
    QTextEdit*    m_siteInstructions;
    QComboBox*    m_status;
    QLabel*       m_errorLabel;
};
