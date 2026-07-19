#include "DashboardWidget.h"
#include "database/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QDate>

DashboardWidget::DashboardWidget(QWidget* parent)
    : QWidget(parent),
      m_totalGuardsVal(nullptr),
      m_presentVal(nullptr),
      m_absentVal(nullptr),
      m_clientsVal(nullptr),
      m_sitesVal(nullptr),
      m_salaryVal(nullptr),
      m_incidentsVal(nullptr)
{
    buildUI();
    refresh();
}

void DashboardWidget::buildUI()
{
    auto* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* content = new QWidget;
    auto* mainLayout = new QVBoxLayout(content);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(0);

    // ---- Header ----
    auto* titleLabel = new QLabel("Dashboard");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    auto* dateLabel = new QLabel(QDate::currentDate().toString("dddd, MMMM d, yyyy"));
    dateLabel->setObjectName("PageSubtitle");
    mainLayout->addWidget(dateLabel);

    mainLayout->addSpacing(28);

    // ---- Stats Grid ----
    auto* statsGrid = new QGridLayout;
    statsGrid->setSpacing(16);

    m_totalGuardsVal = new QLabel("0");
    m_presentVal     = new QLabel("0");
    m_absentVal      = new QLabel("0");
    m_clientsVal     = new QLabel("0");
    m_sitesVal       = new QLabel("0");
    m_salaryVal      = new QLabel("0");
    m_incidentsVal   = new QLabel("0");

    auto* card0 = createStatCard("0", "TOTAL GUARDS");
    m_totalGuardsVal = card0->findChild<QLabel*>("valueLabel");

    auto* card1 = createStatCard("0", "PRESENT TODAY");
    m_presentVal = card1->findChild<QLabel*>("valueLabel");

    auto* card2 = createStatCard("0", "ABSENT TODAY");
    m_absentVal = card2->findChild<QLabel*>("valueLabel");

    auto* card3 = createStatCard("0", "CLIENTS");
    m_clientsVal = card3->findChild<QLabel*>("valueLabel");

    auto* card4 = createStatCard("0", "ACTIVE SITES");
    m_sitesVal = card4->findChild<QLabel*>("valueLabel");

    auto* card5 = createStatCard("0", "SALARY PENDING");
    m_salaryVal = card5->findChild<QLabel*>("valueLabel");

    auto* card6 = createStatCard("0", "TODAY'S INCIDENTS");
    m_incidentsVal = card6->findChild<QLabel*>("valueLabel");

    statsGrid->addWidget(card0, 0, 0);
    statsGrid->addWidget(card1, 0, 1);
    statsGrid->addWidget(card2, 0, 2);
    statsGrid->addWidget(card3, 1, 0);
    statsGrid->addWidget(card4, 1, 1);
    statsGrid->addWidget(card5, 1, 2);
    statsGrid->addWidget(card6, 2, 0);

    mainLayout->addLayout(statsGrid);
    mainLayout->addSpacing(32);

    // ---- Quick Actions ----
    auto* quickLabel = new QLabel("QUICK ACTIONS");
    quickLabel->setObjectName("SectionTitle");
    mainLayout->addWidget(quickLabel);
    mainLayout->addSpacing(12);

    auto* btnRow = new QHBoxLayout;
    btnRow->setSpacing(12);

    auto makeActionBtn = [&](const QString& text, void (DashboardWidget::*signal)()) {
        auto* btn = new QPushButton(text);
        btn->setObjectName("SecondaryButton");
        btn->setFixedHeight(42);
        btn->setCursor(Qt::PointingHandCursor);
        btnRow->addWidget(btn);
        connect(btn, &QPushButton::clicked, this, signal);
    };

    makeActionBtn("Guards",     &DashboardWidget::navigateToGuard);
    makeActionBtn("Attendance", &DashboardWidget::navigateToAttendance);
    makeActionBtn("Salary",     &DashboardWidget::navigateToSalary);
    makeActionBtn("Reports",    &DashboardWidget::navigateToReports);
    makeActionBtn("Backup",     &DashboardWidget::navigateToBackup);

    btnRow->addStretch();
    mainLayout->addLayout(btnRow);

    mainLayout->addStretch();

    scrollArea->setWidget(content);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scrollArea);
}

QWidget* DashboardWidget::createStatCard(const QString& value, const QString& label)
{
    auto* card = new QWidget;
    card->setObjectName("StatCard");
    card->setMinimumHeight(120);

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(22, 18, 22, 18);
    layout->setSpacing(6);

    auto* valLabel = new QLabel(value);
    valLabel->setObjectName("valueLabel");
    valLabel->setStyleSheet(
        "font-size: 32px; font-weight: 700; color: #D4B44C; background: transparent;"
    );
    layout->addWidget(valLabel);

    auto* lblLabel = new QLabel(label);
    lblLabel->setObjectName("StatLabel");
    lblLabel->setStyleSheet(
        "font-size: 11px; font-weight: 700; color: #6B7585; "
        "letter-spacing: 0.5px; background: transparent;"
    );
    layout->addWidget(lblLabel);

    return card;
}

void DashboardWidget::refresh()
{
    auto& db = DatabaseManager::instance();

    if (m_totalGuardsVal) m_totalGuardsVal->setText(QString::number(db.getActiveGuardCount()));
    if (m_presentVal)     m_presentVal->setText(QString::number(db.getPresentTodayCount()));
    if (m_absentVal)      m_absentVal->setText(QString::number(db.getAbsentTodayCount()));
    if (m_clientsVal)     m_clientsVal->setText(QString::number(db.getClientCount()));
    if (m_sitesVal)       m_sitesVal->setText(QString::number(db.getSiteCount()));
    if (m_salaryVal)      m_salaryVal->setText(QString::number(db.getPendingSalaryCount()));
    if (m_incidentsVal)   m_incidentsVal->setText(QString::number(db.getTodayIncidentCount()));
}
