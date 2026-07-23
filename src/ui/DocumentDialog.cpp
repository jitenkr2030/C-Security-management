#include "DocumentDialog.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>

static QString ensureDocumentsDir()
{
    QString dir = QCoreApplication::applicationDirPath() + "/documents";
    QDir().mkpath(dir);
    return dir;
}

DocumentDialog::DocumentDialog(QWidget* parent, int documentId)
    : QDialog(parent), m_documentId(documentId), m_editMode(documentId > 0)
{
    buildUI();
    loadGuardCombo();
    loadClientCombo();
    if (m_editMode) loadDocumentData();
}

void DocumentDialog::buildUI()
{
    setWindowTitle(m_editMode ? "Edit Document" : "Upload Document");
    setMinimumSize(520, 500);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel(m_editMode ? "EDIT DOCUMENT" : "UPLOAD DOCUMENT");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* formGroup = new QGroupBox("Document Details");
    auto* form = new QFormLayout(formGroup);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_guardCombo = new QComboBox;
    m_guardCombo->addItem("-- None --", 0);
    form->addRow("Guard:", m_guardCombo);

    m_clientCombo = new QComboBox;
    m_clientCombo->addItem("-- None --", 0);
    form->addRow("Client:", m_clientCombo);

    m_typeCombo = new QComboBox;
    m_typeCombo->setEditable(true);
    m_typeCombo->addItems({"Aadhaar Card", "PAN Card", "Resume", "Police Verification",
                           "Medical Certificate", "Training Certificate", "Contract",
                           "Insurance", "ID Card", "Photo", "Other"});
    form->addRow("Document Type *:", m_typeCombo);

    m_fileName = new QLineEdit;
    m_fileName->setPlaceholderText("File name");
    form->addRow("File Name *:", m_fileName);

    auto* fileRow = new QHBoxLayout;
    m_filePath = new QLineEdit;
    m_filePath->setPlaceholderText("Click Browse to select a file");
    m_filePath->setReadOnly(true);
    fileRow->addWidget(m_filePath, 1);

    auto* browseBtn = new QPushButton("Browse");
    browseBtn->setObjectName("SecondaryButton");
    browseBtn->setFixedSize(80, 32);
    browseBtn->setCursor(Qt::PointingHandCursor);
    connect(browseBtn, &QPushButton::clicked, this, &DocumentDialog::browseFile);
    fileRow->addWidget(browseBtn);
    form->addRow("File *:", fileRow);

    // File info label
    m_fileInfo = new QLabel;
    m_fileInfo->setStyleSheet("color: #555E6B; font-size: 11px; font-style: italic;");
    m_fileInfo->setWordWrap(true);
    form->addRow("", m_fileInfo);

    m_desc = new QTextEdit;
    m_desc->setPlaceholderText("Description...");
    m_desc->setMaximumHeight(80);
    form->addRow("Description:", m_desc);

    mainLayout->addWidget(formGroup);
    mainLayout->addStretch();

    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("SecondaryButton");
    cancelBtn->setFixedSize(100, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);
    auto* saveBtn = new QPushButton(m_editMode ? "Update" : "Upload");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(100, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &DocumentDialog::saveDocument);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void DocumentDialog::loadGuardCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT id, guard_code, full_name FROM Guards ORDER BY full_name");
    while (q.next()) m_guardCombo->addItem(q.value("full_name").toString() + " (" + q.value("guard_code").toString() + ")", q.value("id").toInt());
}

void DocumentDialog::loadClientCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT id, client_name FROM Clients ORDER BY client_name");
    while (q.next()) m_clientCombo->addItem(q.value("client_name").toString(), q.value("id").toInt());
}

void DocumentDialog::loadDocumentData()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT * FROM Documents WHERE id = :id", {{":id", m_documentId}});
    if (!q.next()) return;
    int guardId = q.value("guard_id").toInt();
    for (int i = 0; i < m_guardCombo->count(); ++i) if (m_guardCombo->itemData(i).toInt() == guardId) { m_guardCombo->setCurrentIndex(i); break; }
    int clientId = q.value("client_id").toInt();
    for (int i = 0; i < m_clientCombo->count(); ++i) if (m_clientCombo->itemData(i).toInt() == clientId) { m_clientCombo->setCurrentIndex(i); break; }
    m_typeCombo->setCurrentText(q.value("document_type").toString());
    m_fileName->setText(q.value("file_name").toString());
    m_filePath->setText(q.value("file_path").toString());

    // Show file info
    QFileInfo fi(q.value("file_path").toString());
    if (fi.exists()) {
        double sizeKB = fi.size() / 1024.0;
        m_fileInfo->setText(QString("Stored: %1 | Size: %2 KB | Modified: %3")
            .arg(fi.fileName())
            .arg(sizeKB, 0, 'f', 1)
            .arg(fi.lastModified().toString("yyyy-MM-dd HH:mm")));
    } else {
        m_fileInfo->setText("Original file not found at stored path");
        m_fileInfo->setStyleSheet("color: #E85454; font-size: 11px; font-style: italic;");
    }

    m_desc->setPlainText(q.value("description").toString());
}

void DocumentDialog::browseFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select File", QString(),
        "Documents (*.pdf *.doc *.docx *.txt *.xls *.xlsx *.ppt *.pptx);;"
        "Images (*.png *.jpg *.jpeg *.bmp *.gif);;"
        "All Files (*)");
    if (filePath.isEmpty()) return;

    QFileInfo fi(filePath);
    if (m_fileName->text().trimmed().isEmpty()) {
        m_fileName->setText(fi.fileName());
    }

    // Copy file to managed documents folder
    QString docsDir = ensureDocumentsDir();
    QString newName = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_") + fi.fileName();
    QString destPath = docsDir + "/" + newName;

    if (QFile::copy(filePath, destPath)) {
        m_filePath->setText(destPath);

        double sizeKB = fi.size() / 1024.0;
        m_fileInfo->setText(QString("Copied: %1 | Size: %2 KB")
            .arg(fi.fileName())
            .arg(sizeKB, 0, 'f', 1));
        m_fileInfo->setStyleSheet("color: #4ADE80; font-size: 11px; font-style: italic;");
    } else {
        // Fallback: store original path
        m_filePath->setText(filePath);

        double sizeKB = fi.size() / 1024.0;
        m_fileInfo->setText(QString("Stored original: %1 | Size: %2 KB")
            .arg(fi.fileName())
            .arg(sizeKB, 0, 'f', 1));
        m_fileInfo->setStyleSheet("color: #FBBF24; font-size: 11px; font-style: italic;");
    }
}

void DocumentDialog::saveDocument()
{
    if (m_fileName->text().trimmed().isEmpty()) { m_errorLabel->setText("File name is required."); m_errorLabel->show(); return; }
    if (m_filePath->text().trimmed().isEmpty()) { m_errorLabel->setText("Please browse and select a file."); m_errorLabel->show(); return; }
    m_errorLabel->hide();

    auto& db = DatabaseManager::instance();
    QVariantMap data;
    data[":gid"] = m_guardCombo->currentData().toInt() > 0 ? m_guardCombo->currentData() : QVariant();
    data[":cid"] = m_clientCombo->currentData().toInt() > 0 ? m_clientCombo->currentData() : QVariant();
    data[":type"] = m_typeCombo->currentText();
    data[":fname"] = m_fileName->text().trimmed();
    data[":fpath"] = m_filePath->text().trimmed();
    data[":desc"] = m_desc->toPlainText().trimmed();

    bool ok;
    if (m_editMode) {
        data[":id"] = m_documentId;
        ok = db.executeNonQuery("UPDATE Documents SET guard_id=:gid, client_id=:cid, document_type=:type, file_name=:fname, file_path=:fpath, description=:desc WHERE id=:id", data);
    } else {
        ok = db.executeNonQuery("INSERT INTO Documents (guard_id, client_id, document_type, file_name, file_path, description) VALUES (:gid, :cid, :type, :fname, :fpath, :desc)", data);
    }
    if (ok) accept();
    else { m_errorLabel->setText("Failed to save document."); m_errorLabel->show(); }
}
