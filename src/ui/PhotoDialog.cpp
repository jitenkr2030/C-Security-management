#include "PhotoDialog.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QPixmap>
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>

static QString ensurePhotosDir()
{
    QString dir = QCoreApplication::applicationDirPath() + "/photos";
    QDir().mkpath(dir);
    return dir;
}

PhotoDialog::PhotoDialog(QWidget* parent, int photoId)
    : QDialog(parent), m_photoId(photoId), m_editMode(photoId > 0)
{
    buildUI();
    loadSiteCombo();
    if (m_editMode) loadPhotoData();
}

void PhotoDialog::buildUI()
{
    setWindowTitle(m_editMode ? "Edit Photo" : "Upload Photo");
    setMinimumSize(500, 560);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel(m_editMode ? "EDIT PHOTO" : "UPLOAD PHOTO");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* formGroup = new QGroupBox("Photo Details");
    auto* form = new QFormLayout(formGroup);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_categoryCombo = new QComboBox;
    m_categoryCombo->addItems({"Guard Photo", "Site Photo", "Incident Evidence",
                               "Training", "Uniform", "Equipment", "ID Card", "Other"});
    form->addRow("Category:", m_categoryCombo);

    m_titleEdit = new QLineEdit;
    m_titleEdit->setPlaceholderText("Photo title or description");
    form->addRow("Title:", m_titleEdit);

    auto* fileRow = new QHBoxLayout;
    m_filePath = new QLineEdit;
    m_filePath->setPlaceholderText("Click Browse to select photo");
    m_filePath->setReadOnly(true);
    fileRow->addWidget(m_filePath, 1);
    auto* browseBtn = new QPushButton("Browse");
    browseBtn->setObjectName("SecondaryButton");
    browseBtn->setFixedSize(80, 32);
    browseBtn->setCursor(Qt::PointingHandCursor);
    connect(browseBtn, &QPushButton::clicked, this, &PhotoDialog::browsePhoto);
    fileRow->addWidget(browseBtn);
    form->addRow("File *:", fileRow);

    // Preview
    m_preview = new QLabel("No image selected");
    m_preview->setStyleSheet("background-color: #1E2530; border: 1px solid #2A3545; border-radius: 6px; color: #6B7585; font-size: 11px;");
    m_preview->setAlignment(Qt::AlignCenter);
    m_preview->setFixedSize(200, 150);
    form->addRow("Preview:", m_preview);

    m_relatedType = new QComboBox;
    m_relatedType->addItems({"None", "Guard", "Site", "Incident", "Training"});
    form->addRow("Related To:", m_relatedType);

    m_relatedId = new QLineEdit;
    m_relatedId->setPlaceholderText("ID of related record");
    form->addRow("Related ID:", m_relatedId);

    m_siteCombo = new QComboBox;
    m_siteCombo->addItem("-- None --", 0);
    form->addRow("Site:", m_siteCombo);

    m_takenDate = new QDateEdit;
    m_takenDate->setCalendarPopup(true);
    m_takenDate->setDisplayFormat("yyyy-MM-dd");
    m_takenDate->setDate(QDate::currentDate());
    form->addRow("Taken Date:", m_takenDate);

    m_uploadedBy = new QLineEdit;
    m_uploadedBy->setPlaceholderText("Your name");
    form->addRow("Uploaded By:", m_uploadedBy);

    m_notes = new QTextEdit;
    m_notes->setPlaceholderText("Notes...");
    m_notes->setMaximumHeight(50);
    form->addRow("Notes:", m_notes);

    mainLayout->addWidget(formGroup, 1);

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
    connect(saveBtn, &QPushButton::clicked, this, &PhotoDialog::savePhoto);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void PhotoDialog::loadSiteCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT id, site_name FROM Sites ORDER BY site_name");
    while (q.next()) m_siteCombo->addItem(q.value("site_name").toString(), q.value("id").toInt());
}

void PhotoDialog::browsePhoto()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select Photo", QString(),
        "Images (*.png *.jpg *.jpeg *.bmp *.gif);;All Files (*)");
    if (filePath.isEmpty()) return;

    QString photosDir = ensurePhotosDir();
    QFileInfo fi(filePath);
    QString newName = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_") + fi.fileName();
    QString destPath = photosDir + "/" + newName;

    if (QFile::copy(filePath, destPath)) {
        m_filePath->setText(destPath);
    } else {
        m_filePath->setText(filePath);
    }
    if (m_titleEdit->text().trimmed().isEmpty()) {
        m_titleEdit->setText(fi.baseName());
    }
    updatePreview();
}

void PhotoDialog::updatePreview()
{
    QString path = m_filePath->text();
    if (path.isEmpty()) {
        m_preview->clear();
        m_preview->setText("No image selected");
        return;
    }
    QPixmap pixmap(path);
    if (!pixmap.isNull()) {
        m_preview->setPixmap(pixmap.scaled(200, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        m_preview->setText("Cannot preview");
    }
}

void PhotoDialog::loadPhotoData()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT * FROM Photos WHERE id = :id", {{":id", m_photoId}});
    if (!q.next()) return;

    m_categoryCombo->setCurrentText(q.value("category").toString());
    m_titleEdit->setText(q.value("title").toString());
    m_filePath->setText(q.value("file_path").toString());
    updatePreview();
    m_relatedType->setCurrentText(q.value("related_type").toString());
    m_relatedId->setText(q.value("related_id").toString());
    int siteId = q.value("site_id").toInt();
    for (int i = 0; i < m_siteCombo->count(); ++i)
        if (m_siteCombo->itemData(i).toInt() == siteId) { m_siteCombo->setCurrentIndex(i); break; }
    m_takenDate->setDate(QDate::fromString(q.value("taken_date").toString(), "yyyy-MM-dd"));
    m_uploadedBy->setText(q.value("uploaded_by").toString());
    m_notes->setPlainText(q.value("notes").toString());
}

void PhotoDialog::savePhoto()
{
    if (m_filePath->text().trimmed().isEmpty()) { m_errorLabel->setText("Please select a photo file."); m_errorLabel->show(); return; }
    m_errorLabel->hide();

    auto& db = DatabaseManager::instance();
    QVariantMap data;
    data[":cat"] = m_categoryCombo->currentText();
    data[":title"] = m_titleEdit->text().trimmed();
    data[":path"] = m_filePath->text().trimmed();
    data[":rtype"] = m_relatedType->currentText() == "None" ? QVariant() : m_relatedType->currentText();
    data[":rid"] = m_relatedId->text().trimmed().toInt() > 0 ? m_relatedId->text().trimmed().toInt() : QVariant();
    data[":sid"] = m_siteCombo->currentData().toInt() > 0 ? m_siteCombo->currentData() : QVariant();
    data[":taken"] = m_takenDate->date().toString("yyyy-MM-dd");
    data[":by"] = m_uploadedBy->text().trimmed();
    data[":notes"] = m_notes->toPlainText().trimmed();

    bool ok;
    if (m_editMode) {
        data[":id"] = m_photoId;
        ok = db.executeNonQuery("UPDATE Photos SET category=:cat, title=:title, file_path=:path, related_type=:rtype, related_id=:rid, site_id=:sid, taken_date=:taken, uploaded_by=:by, notes=:notes WHERE id=:id", data);
    } else {
        ok = db.executeNonQuery("INSERT INTO Photos (category, title, file_path, related_type, related_id, site_id, taken_date, uploaded_by, notes) VALUES (:cat, :title, :path, :rtype, :rid, :sid, :taken, :by, :notes)", data);
    }

    if (ok) accept();
    else { m_errorLabel->setText("Failed to save photo record."); m_errorLabel->show(); }
}
