#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>

class SettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsWidget(QWidget* parent = nullptr);
    void refresh();
private slots:
    void loadSettings();
    void saveSettings();
private:
    void buildUI();

    // Company
    QLineEdit* m_companyName;
    QLineEdit* m_companyAddress;
    QLineEdit* m_companyPhone;
    QLineEdit* m_companyEmail;

    // Salary
    QDoubleSpinBox* m_pfRate;
    QDoubleSpinBox* m_esicRate;
    QDoubleSpinBox* m_ptDeduction;
    QDoubleSpinBox* m_overtimeRate;
    QSpinBox* m_workingDays;

    // Leave
    QSpinBox* m_casualLeave;
    QSpinBox* m_sickLeave;
    QSpinBox* m_earnedLeave;

    QLabel* m_statusLabel;
};
