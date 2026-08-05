#pragma once
#include <QString>
#include "database/DatabaseManager.h"

class PermissionManager
{
public:
    static bool canView(const QString& role, const QString& module) {
        return DatabaseManager::instance().hasPermission(role, module, "can_view");
    }
    static bool canCreate(const QString& role, const QString& module) {
        return DatabaseManager::instance().hasPermission(role, module, "can_create");
    }
    static bool canEdit(const QString& role, const QString& module) {
        return DatabaseManager::instance().hasPermission(role, module, "can_edit");
    }
    static bool canDelete(const QString& role, const QString& module) {
        return DatabaseManager::instance().hasPermission(role, module, "can_delete");
    }
};
