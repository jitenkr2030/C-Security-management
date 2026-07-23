#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QDateEdit>
#include <QLabel>
#include <QTableWidget>

class TrainingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TrainingDialog(QWidget* parent = nullptr, int trainingId = -1);
private slots:
    void saveTraining();
    void addParticipant();
    void removeParticipant();
private:
    void buildUI();
    void loadTrainingData();
    void loadParticipants();
    void loadGuardCombo();
    int m_trainingId;
    bool m_editMode;
    QLineEdit* m_nameEdit;
    QComboBox* m_typeCombo;
    QTextEdit* m_descEdit;
    QLineEdit* m_trainerEdit;
    QDateEdit* m_startDate;
    QDateEdit* m_endDate;
    QLineEdit* m_locationEdit;
    QTextEdit* m_notes;
    QComboBox* m_guardCombo;
    QTableWidget* m_partTable;
    QLabel* m_errorLabel;
    QLabel* m_partCount;
};
