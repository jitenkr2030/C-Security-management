#include "LoginDialog.h"
#include "database/DatabaseManager.h"

#include <QApplication>
#include <QScreen>
#include <QGraphicsDropShadowEffect>
#include <QSpacerItem>

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent)
{
    buildUI();
    setObjectName("LoginDialog");
    setWindowTitle("Security Guard Manager — Login");
    setFixedSize(420, 520);
    setModal(true);

    // Center on screen
    if (auto screen = QApplication::primaryScreen()) {
        auto geo = screen->geometry();
        move((geo.width() - width()) / 2, (geo.height() - height()) / 2);
    }
}

void LoginDialog::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(48, 40, 48, 40);
    mainLayout->setSpacing(0);

    // ---- Logo / Title area ----
    mainLayout->addSpacing(30);

    auto* iconLabel = new QLabel("SG");
    iconLabel->setObjectName("StatValue");
    iconLabel->setStyleSheet(
        "font-size: 36px; font-weight: 800; color: #D4B44C; "
        "background-color: #1A2233; border-radius: 20px; "
        "padding: 16px 22px; border: 2px solid #2A3545;"
    );
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedWidth(80);
    mainLayout->addWidget(iconLabel, 0, Qt::AlignCenter);

    mainLayout->addSpacing(24);

    auto* titleLabel = new QLabel("SECURITY GUARD\nMANAGEMENT");
    titleLabel->setObjectName("LoginTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    auto* subtitleLabel = new QLabel("Enter your credentials to continue");
    subtitleLabel->setObjectName("LoginSubtitle");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(subtitleLabel);

    mainLayout->addSpacing(36);

    // ---- Username ----
    auto* userLabel = new QLabel("USERNAME");
    userLabel->setObjectName("SectionTitle");
    mainLayout->addWidget(userLabel);
    mainLayout->addSpacing(6);

    m_usernameEdit = new QLineEdit;
    m_usernameEdit->setPlaceholderText("Enter username");
    m_usernameEdit->setText("admin");
    mainLayout->addWidget(m_usernameEdit);

    mainLayout->addSpacing(18);

    // ---- Password ----
    auto* passLabel = new QLabel("PASSWORD");
    passLabel->setObjectName("SectionTitle");
    mainLayout->addWidget(passLabel);
    mainLayout->addSpacing(6);

    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setPlaceholderText("Enter password");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    mainLayout->addWidget(m_passwordEdit);

    mainLayout->addSpacing(8);

    // ---- Error label ----
    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-size: 12px; font-weight: 600;");
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setWordWrap(true);
    mainLayout->addWidget(m_errorLabel);

    mainLayout->addStretch();

    // ---- Login button ----
    m_loginBtn = new QPushButton("SIGN IN");
    m_loginBtn->setObjectName("PrimaryButton");
    m_loginBtn->setFixedHeight(46);
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    mainLayout->addWidget(m_loginBtn);

    // ---- Connections ----
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::attemptLogin);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::attemptLogin);
    connect(m_usernameEdit, &QLineEdit::returnPressed, [this]() {
        m_passwordEdit->setFocus();
    });
}

void LoginDialog::attemptLogin()
{
    m_errorLabel->clear();

    QString user = m_usernameEdit->text().trimmed();
    QString pass = m_passwordEdit->text();

    if (user.isEmpty() || pass.isEmpty()) {
        m_errorLabel->setText("Please enter both username and password.");
        return;
    }

    int userId = -1;
    QString role;

    if (DatabaseManager::instance().validateUser(user, pass, role, userId)) {
        m_username = user;
        m_role = role;
        m_userId = userId;
        accept();
    } else {
        m_errorLabel->setText("Invalid username or password.");
        m_passwordEdit->clear();
        m_passwordEdit->setFocus();
    }
}

QString LoginDialog::username() const { return m_username; }
QString LoginDialog::role() const { return m_role; }
int LoginDialog::userId() const { return m_userId; }
