#include "StyleManager.h"
#include <QFile>

QString StyleManager::loadStyleSheet()
{
    QFile file(":/styles/dark_theme.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        return QString::fromUtf8(file.readAll());
    }
    return {};
}
