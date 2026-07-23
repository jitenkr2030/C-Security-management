#include "IncidentDialog.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QScrollArea>
#include <QFileDialog>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>

static QString ensureEvidenceDir()
{
    QString dir = QCoreApplication::applicationDirPath() + "/evidence";
    QDir().mkpath(dir);
    return dir;
}

IncidentDialog::IncidentDialog(QWidget* parent, int incidentId)
    : QDialog(parent), m_incidentId(incidentId), m_editMode(incidentId > 0)
{
    buildUI();
    loadSiteCombo();
    loadGuardCombo();
    if (m_editMode) loadIncidentData();
}

void IncidentDialog::buildUI()
{
    setWindowTitle(m_editMode ? "Edit Incident" : "Report Incident");
    setMinimumSize(580, 620);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(12);

    auto* titleLabel = new QLabel(m_editMode ? "EDIT INCIDENT" : "REPORT NEW INCIDENT");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto* formWidget = new QWidget;
    auto* form = new QFormLayout(formWidget);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_codeEdit = new QLineEdit;
    m_codeEdit->setPlaceholderText("e.g. INC-001");
    form->addRow("Incident Code *:", m_codeEdit);

    m_typeCombo = new QComboBox;
    m_typeCombo->setEditable(true);
    m_typeCombo->addItems({"Theft", "Fire", "Accident", "Trespass", "Assault",
                           "Equipment Failure", "Medical Emergency", "Vandalism",
                           "Power Failure", "Water Leak", "Other"});
    form->addRow("Type *:", m_typeCombo);

    m_severityCombo = new QComboBox;
    m_severityCombo->addItems({"Low", "Medium", "High", "Critical"});
    form->addRow("Severity:", m_severityCombo);

    m_siteCombo = new QComboBox;
    m_siteCombo->addItem("-- Select Site --", 0);
    form->addRow("Site:", m_siteCombo);

    m_guardCombo = new QComboBox;
    m_guardCombo->addItem("-- Select Guard (optional) --", 0);
    form->addRow("Guard:", m_guardCombo);

    m_dateTime = new QDateTimeEdit;
    m_dateTime->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_dateTime->setDateTime(QDateTime::currentDateTime());
    m_dateTime->setCalendarPopup(true);
    form->addRow("Date/Time *:", m_dateTime);

    m_description = new QTextEdit;
    m_description->setPlaceholderText("Describe the incident...");
    m_description->setMaximumHeight(80);
    form->addRow("Description *:", m_description);

    m_actionTaken = new QTextEdit;
    m_actionTaken->setPlaceholderText("Action taken...");
    m_actionTaken->setMaximumHeight(60);
    form->addRow("Action Taken:", m_actionTaken);

    m_reportedBy = new QLineEdit;
    m_reportedBy->setPlaceholderText("Name of person reporting");
    form->addRow("Reported By:", m_reportedBy);

    m_witnessName = new QLineEdit;
    m_witnessName->setPlaceholderText("Witness name");
    form->addRow("Witness:", m_witnessName);

    m_witnessContact = new QLineEdit;
    m_witnessContact->setPlaceholderText("Witness contact");
    form->addRow("Witness Contact:", m_witnessContact);

    m_statusCombo = new QComboBox;
    m_statusCombo->addItems({"Open", "Under Investigation", "Resolved", "Closed"});
    form->addRow("Status:", m_statusCombo);

    m_resolution = new QTextEdit;
    m_resolution->setPlaceholderText("Resolution details...");
    m_resolution->setMaximumHeight(60);
    form->addRow("Resolution:", m_resolution);

    // Photo attachment
    auto* photoRow = new QHBoxLayout;
    m_photoPath = new QLineEdit;
    m_photoPath->setPlaceholderText("Attach photo...");
    m_photoPath->setReadOnly(true);
    photoRow->addWidget(m_photoPath, 1);

    auto* photoBrowseBtn = new QPushButton("Browse");
    photoBrowseBtn->setObjectName("SecondaryButton");
    photoBrowseBtn->setFixedSize(80, 32);
    photoBrowseBtn->setCursor(Qt::PointingHandCursor);
    connect(photoBrowseBtn, &QPushButton::clicked, this, &IncidentDialog::browsePhoto);
    photoRow->addWidget(photoBrowseBtn);

    auto* photoClearBtn = new QPushButton("Clear");
    photoClearBtn->setFixedSize(60, 32);
    photoClearBtn->setCursor(Qt::PointingHandCursor);
    connect(photoClearBtn, &QPushButton::clicked, this, [this]() {
        m_photoPath->clear();
        m_photoPreview->clear();
        m_photoPreview->setText("No photo");
        m_photoPreview->setFixedSize(0, 0);
    });
    photoRow->addWidget(photoClearBtn);
    form->addRow("Photo:", photoRow);

    // Photo preview
    m_photoPreview = new QLabel("No photo");
    m_photoPreview->setStyleSheet("background-color: #1E2530; border: 1px solid #2A3545; border-radius: 6px; color: #6B7585; font-size: 11px;");
    m_photoPreview->setAlignment(Qt::AlignCenter);
    m_photoPreview->setFixedSize(0, 0);
    form->addRow("", m_photoPreview);

    // Document attachment
    auto* docRow = new QHBoxLayout;
    m_docPath = new QLineEdit;
    m_docPath->setPlaceholderText("Attach supporting document...");
    m_docPath->setReadOnly(true);
    docRow->addWidget(m_docPath, 1);

    auto* docBrowseBtn = new QPushButton("Browse");
    docBrowseBtn->setObjectName("SecondaryButton");
    docBrowseBtn->setFixedSize(80, 32);
    docBrowseBtn->setCursor(Qt::PointingHandCursor);
    connect(docBrowseBtn, &QPushButton::clicked, this, &IncidentDialog::browseDocument);
    docRow->addWidget(docBrowseBtn);

    auto* docClearBtn = new QPushButton("Clear");
    docClearBtn->setFixedSize(60, 32);
    docClearBtn->setCursor(Qt::PointingHandCursor);
    connect(docClearBtn, &QPushButton::clicked, this, [this]() { m_docPath->clear(); });
    docRow->addWidget(docClearBtn);
    form->addRow("Document:", docRow);

    scrollArea->setWidget(formWidget);
    mainLayout->addWidget(scrollArea, 1);

    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("SecondaryButton");
    cancelBtn->setFixedSize(100, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);
    auto* saveBtn = new QPushButton(m_editMode ? "Update" : "Report Incident");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(160, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &IncidentDialog::saveIncident);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void IncidentDialog::browsePhoto()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select Photo",
        QString(), "Images (*.png *.jpg *.jpeg *.bmp *.gif);;All Files (*)");
    if (filePath.isEmpty()) return;

    // Copy to evidence folder
    QString evidenceDir = ensureEvidenceDir();
    QFileInfo fi(filePath);
    QString newName = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_") + fi.fileName();
    QString destPath = evidenceDir + "/" + newName;

    if (QFile::copy(filePath, destPath)) {
        m_photoPath->setText(destPath);
        updatePhotoPreview();
    } else {
        // If copy fails, store original path
        m_photoPath->setText(filePath);
        updatePhotoPreview();
    }
}

void IncidentDialog::browseDocument()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select Document",
        QString(), "Documents (*.pdf *.doc *.docx *.txt *.xls *.xlsx);;Images (*.png *.jpg *.jpeg);;All Files (*)");
    if (filePath.isEmpty()) return;

    // Copy to evidence folder
    QString evidenceDir = ensureEvidenceDir();
    QFileInfo fi(filePath);
    QString newName = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_") + fi.fileName();
    QString destPath = evidenceDir + "/" + newName;

    if (QFile::copy(filePath, destPath)) {
        m_docPath->setText(destPath);
    } else {
        m_docPath->setText(filePath);
    }
}

void IncidentDialog::updatePhotoPreview()
{
    QString path = m_photoPath->text();
    if (path.isEmpty()) {
        m_photoPreview->clear();
        m_photoPreview->setText("No photo");
        m_photoPreview->setFixedSize(0, 0);
        return;
    }

    QPixmap pixmap(path);
    if (!pixmap.isNull()) {
        m_photoPreview->setFixedSize(200, 150);
        m_photoPreview->setPixmap(pixmap.scaled(200, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        m_photoPreview->setFixedSize(200, 30);
        m_photoPreview->setText("Cannot preview: " + QFileInfo(path).fileName());
    }
}

void IncidentDialog::loadSiteCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT id, site_name FROM Sites ORDER BY site_name");
    while (q.next()) m_siteCombo->addItem(q.value("site_name").toString(), q.value("id").toInt());
}

void IncidentDialog::loadGuardCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT id, guard_code, full_name FROM Guards WHERE status = 'Active' ORDER BY full_name");
    while (q.next()) m_guardCombo->addItem(q.value("full_name").toString() + " (" + q.value("guard_code").toString() + ")", q.value("id").toInt());
}

void IncidentDialog::loadIncidentData()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT * FROM Incidents WHERE id = :id", {{":id", m_incidentId}});
    if (!q.next()) return;

    m_codeEdit->setText(q.value("incident_code").toString());
    m_typeCombo->setCurrentText(q.value("incident_type").toString());
    m_severityCombo->setCurrentText(q.value("severity").toString());

    int siteId = q.value("site_id").toInt();
    for (int i = 0; i < m_siteCombo->count(); ++i)
        if (m_siteCombo->itemData(i).toInt() == siteId) { m_siteCombo->setCurrentIndex(i); break; }
    int guardId = q.value("guard_id").toInt();
    for (int i = 0; i < m_guardCombo->count(); ++i)
        if (m_guardCombo->itemData(i).toInt() == guardId) { m_guardCombo->setCurrentIndex(i); break; }

    m_dateTime->setDateTime(QDateTime::fromString(q.value("date_time").toString(), "yyyy-MM-dd HH:mm"));
    m_description->setPlainText(q.value("description").toString());
    m_actionTaken->setPlainText(q.value("action_taken").toString());
    m_reportedBy->setText(q.value("reported_by").toString());
    m_witnessName->setText(q.value("witness_name").toString());
    m_witnessContact->setText(q.value("witness_contact").toString());
    m_statusCombo->setCurrentText(q.value("status").toString());
    m_resolution->setPlainText(q.value("resolution").toString());

    // Load photo and document paths
    QString photoPath = q.value("photo_path").toString();
    if (!photoPath.isEmpty()) {
        m_photoPath->setText(photoPath);
        updatePhotoPreview();
    }

    QString docPath = q.value("document_path").toString();
    if (!docPath.isEmpty()) {
        m_docPath->setText(docPath);
    }
}

void IncidentDialog::saveIncident()
{
    if (m_codeEdit->text().trimmed().isEmpty()) { m_errorLabel->setText("Incident code is required."); m_errorLabel->show(); return; }
    if (m_description->toPlainText().trimmed().isEmpty()) { m_errorLabel->setText("Description is required."); m_errorLabel->show(); return; }
    m_errorLabel->hide();

    auto& db = DatabaseManager::instance();
    QVariantMap data;
    data[":code"] = m_codeEdit->text().trimmed().toUpper();
    data[":type"] = m_typeCombo->currentText();
    data[":severity"] = m_severityCombo->currentText();
    data[":sid"] = m_siteCombo->currentData().toInt() > 0 ? m_siteCombo->currentData() : QVariant();
    data[":gid"] = m_guardCombo->currentData().toInt() > 0 ? m_guardCombo->currentData() : QVariant();
    data[":dt"] = m_dateTime->dateTime().toString("yyyy-MM-dd HH:mm");
    data[":desc"] = m_description->toPlainText().trimmed();
    data[":action"] = m_actionTaken->toPlainText().trimmed();
    data[":by"] = m_reportedBy->text().trimmed();
    data[":wname"] = m_witnessName->text().trimmed();
    data[":wcontact"] = m_witnessContact->text().trimmed();
    data[":status"] = m_statusCombo->currentText();
    data[":res"] = m_resolution->toPlainText().trimmed();
    data[":photo"] = m_photoPath->text().trimmed();
    data[":doc"] = m_docPath->text().trimmed();

    bool ok;
    if (m_editMode) {
        data[":id"] = m_incidentId;
        ok = db.executeNonQuery(
            "UPDATE Incidents SET incident_code=:code, incident_type=:type, severity=:severity, "
            "site_id=:sid, guard_id=:gid, date_time=:dt, description=:desc, action_taken=:action, "
            "reported_by=:by, witness_name=:wname, witness_contact=:wcontact, status=:status, "
            "resolution=:res, photo_path=:photo, document_path=:doc WHERE id=:id", data);
    } else {
        ok = db.executeNonQuery(
            "INSERT INTO Incidents (incident_code, incident_type, severity, site_id, guard_id, "
            "date_time, description, action_taken, reported_by, witness_name, witness_contact, "
            "status, resolution, photo_path, document_path) "
            "VALUES (:code, :type, :severity, :sid, :gid, :dt, :desc, :action, :by, :wname, "
            ":wcontact, :status, :res, :photo, :doc)", data);
    }

    if (ok) accept();
    else { m_errorLabel->setText("Failed to save. Code may already exist."); m_errorLabel->show(); }
}
