#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

class LicenseDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LicenseDialog(QWidget* parent = nullptr);
    static bool isLicenseValid();
private slots:
    void activateLicense();
private:
    void buildUI();
    QLineEdit* m_licenseEdit;
    QLabel* m_errorLabel;
    QLabel* m_statusLabel;
};
