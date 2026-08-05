#pragma once
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QLabel>

class FineDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FineDialog(QWidget* parent = nullptr, int fineId = -1);
private slots:
    void saveFine();
private:
    void buildUI();
    void loadFineData();
    void loadGuardCombo();
    int m_fineId;
    bool m_editMode;
    QComboBox* m_guardCombo;
    QComboBox* m_typeCombo;
    QTextEdit* m_reason;
    QDoubleSpinBox* m_amount;
    QDateEdit* m_fineDate;
    QComboBox* m_monthCombo;
    QSpinBox* m_yearSpin;
    QComboBox* m_statusCombo;
    QLineEdit* m_approvedBy;
    QTextEdit* m_notes;
    QLabel* m_errorLabel;
};
