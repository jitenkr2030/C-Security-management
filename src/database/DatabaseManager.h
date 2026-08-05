#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariantMap>
#include <QVariantList>
#include <QString>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <optional>

class DatabaseManager
{
public:
    static DatabaseManager& instance();

    bool initialize(const QString& dbPath = {});
    bool isOpen() const;
    QString databasePath() const;

    // Generic query helpers
    QSqlQuery execute(const QString& sql, const QVariantMap& params = {});
    bool hasPermission(const QString& role, const QString& module, const QString& action);
    QSqlQuery getPermissions(const QString& role);
    bool executeNonQuery(const QString& sql, const QVariantMap& params = {});
    int lastInsertId() const;

    // Table checks
    bool tableExists(const QString& tableName) const;

    // Authentication
    bool validateUser(const QString& username, const QString& password,
                      QString& role, int& userId);

    // Guards
    QVariantList getAllGuards();
    QVariantMap getGuardById(int id);
    bool insertGuard(const QVariantMap& data);
    bool updateGuard(int id, const QVariantMap& data);
    bool deleteGuard(int id);
    int getGuardCount();
    int getActiveGuardCount();
    int getPresentTodayCount();
    int getAbsentTodayCount();

    // Dashboard stats
    int getClientCount();
    int getSiteCount();
    int getPendingSalaryCount();
    int getTodayIncidentCount();

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    void createTables();
    void seedDefaultAdmin();
    void seedDefaultPermissions();

    QSqlDatabase m_db;
    QString m_dbPath;
};
