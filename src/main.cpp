#include <QApplication>
#include <QDir>
#include <QDebug>

#include "database/DatabaseManager.h"
#include "ui/StyleManager.h"
#include "ui/LicenseDialog.h"
#include "ui/LoginDialog.h"
#include "ui/MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Security Guard Manager");
    app.setOrganizationName("SecurityGuardManagement");
    app.setApplicationVersion("1.0.0");

    QString appDir = QCoreApplication::applicationDirPath();
    QStringList dirs = {"photos", "documents", "reports", "backups"};
    for (const auto& dir : dirs) {
        QDir().mkpath(appDir + "/" + dir);
    }

    app.setStyleSheet(StyleManager::loadStyleSheet());

    if (!DatabaseManager::instance().initialize()) {
        qCritical() << "Failed to initialize database. Exiting.";
        return 1;
    }

    qInfo() << "Database ready at:" << DatabaseManager::instance().databasePath();

    // Check license first
    if (!LicenseDialog::isLicenseValid()) {
        LicenseDialog licenseDialog;
        if (licenseDialog.exec() != QDialog::Accepted) {
            return 0;
        }
    }

    LoginDialog loginDialog;
    if (loginDialog.exec() != QDialog::Accepted) {
        return 0;
    }

    qInfo() << "User logged in:" << loginDialog.username() << "Role:" << loginDialog.role();

    MainWindow mainWindow(loginDialog.username(), loginDialog.role(), loginDialog.userId());
    mainWindow.show();

    return app.exec();
}
