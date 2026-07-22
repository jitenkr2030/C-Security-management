#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QTextEdit>
#include <QLabel>

class EquipmentDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode { ItemMode, IssueMode, ReturnMode };

    explicit EquipmentDialog(Mode mode, QWidget* parent = nullptr, int id = -1);

private slots:
    void save();

private:
    void buildItemUI();
    void buildIssueUI();
    void buildReturnUI();
    void loadItemData();
    void loadGuardCombo();
    void loadEquipmentCombo();

    Mode m_mode;
    int  m_id;

    // Item mode
    QLineEdit*  m_codeEdit;
    QLineEdit*  m_typeEdit;
    QTextEdit*  m_descEdit;
    QLineEdit*  m_serialEdit;
    QDateEdit*  m_purchaseDate;
    QComboBox*  m_conditionCombo;
    QComboBox*  m_statusCombo;
    QTextEdit*  m_notes;

    // Issue mode
    QComboBox*  m_guardCombo;
    QComboBox*  m_equipCombo;
    QComboBox*  m_conditionOut;

    // Return mode
    QComboBox*  m_conditionIn;
    QTextEdit*  m_returnNotes;

    QLabel*     m_errorLabel;
};
