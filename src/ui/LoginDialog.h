#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget* parent = nullptr);

    QString username() const;
    QString role() const;
    int userId() const;

private slots:
    void attemptLogin();

private:
    void buildUI();

    QLineEdit*  m_usernameEdit;
    QLineEdit*  m_passwordEdit;
    QPushButton* m_loginBtn;
    QLabel*     m_errorLabel;

    QString m_username;
    QString m_role;
    int     m_userId = -1;
};
