#include "BackupWidget.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QDateTime>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

BackupWidget::BackupWidget(QWidget* parent) : QWidget(parent)
{
    buildUI();
    loadBackupLog();
}

void BackupWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Backup & Restore");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);
    auto* subtitle = new QLabel("Create database backups and restore from previous backups");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    // Database info
    auto* infoRow = new QHBoxLayout;
    m_dbSizeLabel = new QLabel;
    m_dbSizeLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    infoRow->addWidget(m_dbSizeLabel);
    infoRow->addStretch();
    mainLayout->addLayout(infoRow);

    // Action buttons
    auto* actionRow = new QHBoxLayout;
    actionRow->setSpacing(10);

    auto* backupBtn = new QPushButton("Create Backup");
    backupBtn->setObjectName("PrimaryButton");
    backupBtn->setFixedSize(150, 40);
    backupBtn->setCursor(Qt::PointingHandCursor);
    connect(backupBtn, &QPushButton::clicked, this, &BackupWidget::createBackup);
    actionRow->addWidget(backupBtn);

    auto* restoreBtn = new QPushButton("Restore Backup");
    restoreBtn->setObjectName("SecondaryButton");
    restoreBtn->setFixedSize(150, 40);
    restoreBtn->setCursor(Qt::PointingHandCursor);
    restoreBtn->setStyleSheet(
        "QPushButton { background-color: #3A2A1A; color: #FB923C; border: 1px solid #5A3A1A; "
        "border-radius: 6px; padding: 8px 16px; font-weight: 600; }"
        "QPushButton:hover { background-color: #4A3A2A; }");
    connect(restoreBtn, &QPushButton::clicked, this, &BackupWidget::restoreBackup);
    actionRow->addWidget(restoreBtn);

    actionRow->addStretch();
    mainLayout->addLayout(actionRow);

    // Status
    m_statusLabel = new QLabel;
    m_statusLabel->setStyleSheet("color: #4ADE80; font-size: 13px; font-weight: 600;");
    mainLayout->addWidget(m_statusLabel);

    // Backup log table
    auto* logLabel = new QLabel("Backup History");
    logLabel->setStyleSheet("color: #D4B44C; font-size: 15px; font-weight: 700;");
    mainLayout->addWidget(logLabel);

    m_table = new QTableWidget;
    m_table->setAlternatingRowColors(true);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(true);

    QStringList cols = {"ID", "Backup Path", "Type", "Size", "Created"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);
    m_table->setColumnWidth(1, 400);
    m_table->setColumnWidth(2, 100);
    m_table->setColumnWidth(3, 100);
    m_table->horizontalHeader()->setStretchLastSection(true);

    mainLayout->addWidget(m_table, 1);
}

void BackupWidget::refresh()
{
    loadBackupLog();

    // Update DB size
    QString dbPath = QCoreApplication::applicationDirPath() + "/database.db";
    QFileInfo fi(dbPath);
    if (fi.exists()) {
        double sizeMB = fi.size() / (1024.0 * 1024.0);
        m_dbSizeLabel->setText(QString("Database: %1 | Size: %2 MB")
            .arg(dbPath).arg(sizeMB, 0, 'f', 2));
    }
}

void BackupWidget::createBackup()
{
    QString dbPath = QCoreApplication::applicationDirPath() + "/database.db";
    QFileInfo dbInfo(dbPath);
    if (!dbInfo.exists()) {
        QMessageBox::warning(this, "Error", "Database file not found.");
        return;
    }

    // Create backups directory
    QString backupDir = QCoreApplication::applicationDirPath() + "/backups";
    QDir().mkpath(backupDir);

    // Generate backup filename
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString backupName = "sgms_backup_" + timestamp + ".db";
    QString backupPath = backupDir + "/" + backupName;

    // Also offer to save to user location
    QString userPath = QFileDialog::getSaveFileName(
        this, "Save Backup As",
        QCoreApplication::applicationDirPath() + "/backups/" + backupName,
        "Database Files (*.db);;All Files (*)");

    if (userPath.isEmpty()) {
        // User cancelled, still save to default location
        userPath = backupPath;
    }

    if (QFile::copy(dbPath, userPath)) {
        qint64 fileSize = QFileInfo(userPath).size();

        // Log the backup
        auto& db = DatabaseManager::instance();
        db.executeNonQuery(
            "INSERT INTO BackupLog (backup_path, backup_type, file_size) "
            "VALUES (:path, 'Manual', :size)",
            {{":path", userPath}, {":size", fileSize}}
        );

        m_statusLabel->setStyleSheet("color: #4ADE80; font-size: 13px; font-weight: 600;");
        m_statusLabel->setText(QString("Backup created successfully: %1").arg(userPath));

        loadBackupLog();
    } else {
        m_statusLabel->setStyleSheet("color: #E85454; font-size: 13px; font-weight: 600;");
        m_statusLabel->setText("Failed to create backup. File may already exist at destination.");
    }
}

void BackupWidget::restoreBackup()
{
    auto result = QMessageBox::warning(this, "Restore Backup",
        "WARNING: Restoring a backup will REPLACE your current database.\n\n"
        "The application will need to be restarted after restore.\n\n"
        "Are you sure you want to continue?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (result != QMessageBox::Yes) return;

    QString backupPath = QFileDialog::getOpenFileName(
        this, "Select Backup File",
        QCoreApplication::applicationDirPath() + "/backups",
        "Database Files (*.db);;All Files (*)");

    if (backupPath.isEmpty()) return;

    QFileInfo backupInfo(backupPath);
    if (!backupInfo.exists()) {
        QMessageBox::warning(this, "Error", "Selected backup file not found.");
        return;
    }

    // Create a safety backup first
    QString dbPath = QCoreApplication::applicationDirPath() + "/database.db";
    QString safetyBackup = dbPath + ".before_restore";
    QFile::remove(safetyBackup);
    QFile::copy(dbPath, safetyBackup);

    // Restore
    QFile::remove(dbPath);
    if (QFile::copy(backupPath, dbPath)) {
        m_statusLabel->setStyleSheet("color: #4ADE80; font-size: 13px; font-weight: 600;");
        m_statusLabel->setText("Backup restored successfully. Please restart the application.");
        QMessageBox::information(this, "Restore Complete",
            "Database restored successfully.\n\nPlease restart the application for changes to take effect.");
    } else {
        // Restore safety backup
        QFile::remove(dbPath);
        QFile::copy(safetyBackup, dbPath);
        m_statusLabel->setStyleSheet("color: #E85454; font-size: 13px; font-weight: 600;");
        m_statusLabel->setText("Failed to restore backup.");
    }
}

void BackupWidget::loadBackupLog()
{
    auto& db = DatabaseManager::instance();

    auto cc = db.execute("SELECT COUNT(*) FROM BackupLog");
    int count = 0;
    if (cc.next()) count = cc.value(0).toInt();

    m_table->setRowCount(count);
    m_table->setSortingEnabled(false);

    auto query = db.execute("SELECT * FROM BackupLog ORDER BY created_at DESC");

    int row = 0;
    while (query.next()) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };

        setItem(0, query.value("id").toString());
        setItem(1, query.value("backup_path").toString());
        setItem(2, query.value("backup_type").toString());

        qint64 size = query.value("file_size").toLongLong();
        double sizeMB = size / (1024.0 * 1024.0);
        auto* sizeItem = new QTableWidgetItem(QString::number(sizeMB, 'f', 2) + " MB");
        sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(row, 3, sizeItem);

        setItem(4, query.value("created_at").toString());
        row++;
    }
    m_table->setSortingEnabled(true);
}
