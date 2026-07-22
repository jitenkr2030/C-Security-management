#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QLabel>

class UniformDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode { ItemMode, IssueMode, ReturnMode };

    explicit UniformDialog(Mode mode, QWidget* parent = nullptr, int id = -1);

    int selectedGuardId() const;
    int selectedUniformId() const;

private slots:
    void save();

private:
    void buildItemUI();
    void buildIssueUI();
    void buildReturnUI();
    void loadItemData();
    void loadGuardCombo();
    void loadUniformCombo();

    Mode m_mode;
    int  m_id;

    // Item mode
    QLineEdit*  m_itemType;
    QComboBox*  m_sizeCombo;
    QSpinBox*   m_qtySpin;
    QComboBox*  m_statusCombo;
    QTextEdit*  m_notes;

    // Issue mode
    QComboBox*  m_guardCombo;
    QComboBox*  m_uniformCombo;
    QComboBox*  m_conditionOut;

    // Return mode
    QComboBox*  m_conditionIn;
    QTextEdit*  m_returnNotes;

    QLabel*     m_errorLabel;
};
