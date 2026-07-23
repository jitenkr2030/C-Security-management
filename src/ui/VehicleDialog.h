#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QLabel>
#include <QTimeEdit>

class VehicleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VehicleDialog(QWidget* parent = nullptr, int vehicleId = -1);

private slots:
    void saveVehicle();

private:
    void buildUI();
    void loadVehicleData();
    void loadSiteCombo();

    int  m_vehicleId;
    bool m_editMode;

    QComboBox*  m_siteCombo;
    QLineEdit*  m_vehicleNo;
    QComboBox*  m_vehicleType;
    QLineEdit*  m_driverName;
    QLineEdit*  m_driverMobile;
    QLineEdit*  m_purposeEdit;
    QTimeEdit*  m_entryTime;
    QTextEdit*  m_notes;
    QLabel*     m_errorLabel;
};
