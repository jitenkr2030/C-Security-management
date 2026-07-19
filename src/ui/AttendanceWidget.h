#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QDateEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QMap>

class AttendanceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AttendanceWidget(QWidget* parent = nullptr, int userId = 0);
    void refresh();

private slots:
    void loadDailyAttendance();
    void saveDailyAttendance();
    void markAllPresent();
    void clearAllStatus();
    void updateDailyStats();
    void loadMonthlyRegister();
    void exportMonthlyCSV();

private:
    void buildUI();
    QWidget* buildDailyTab();
    QWidget* buildMonthlyTab();
    QString statusStyleSheet(const QString& status) const;
    QColor statusBgColor(const QString& status) const;
    QColor statusFgColor(const QString& status) const;
    QString statusLetter(const QString& status) const;

    int m_userId;

    // Daily tab
    QDateEdit*    m_dailyDate;
    QComboBox*    m_dailySiteCombo;
    QTableWidget* m_dailyTable;
    QPushButton*  m_saveBtn;
    QPushButton*  m_markAllBtn;
    QPushButton*  m_clearBtn;
    QLabel*       m_statsLabel;
    QLabel*       m_shiftLabel;

    QMap<int, QComboBox*>  m_statusCombos;
    QMap<int, QLineEdit*>  m_notesEdits;

    // Monthly tab
    QComboBox*    m_monthCombo;
    QSpinBox*     m_yearSpin;
    QComboBox*    m_monthlySiteCombo;
    QTableWidget* m_monthlyTable;
    QPushButton*  m_exportBtn;
};
