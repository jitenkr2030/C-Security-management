#include "ComplaintDialog.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QScrollArea>

ComplaintDialog::ComplaintDialog(QWidget* parent, int complaintId)
    : QDialog(parent), m_complaintId(complaintId), m_editMode(complaintId > 0)
{
    buildUI();
    loadClientCombo();
    loadSiteCombo();
    loadGuardCombo();
    if (m_editMode) loadComplaintData();
}

void ComplaintDialog::buildUI()
{
    setWindowTitle(m_editMode ? "Edit Complaint" : "New Complaint / Feedback");
    setMinimumSize(560, 600);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(12);

    auto* titleLabel = new QLabel(m_editMode ? "EDIT COMPLAINT" : "NEW COMPLAINT / FEEDBACK");
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
    m_codeEdit->setPlaceholderText("e.g. CMP-001");
    form->addRow("Complaint Code *:", m_codeEdit);

    m_typeCombo = new QComboBox;
    m_typeCombo->addItems({"Client", "Guard", "Public", "Internal"});
    form->addRow("Type:", m_typeCombo);

    m_categoryCombo = new QComboBox;
    m_categoryCombo->setEditable(true);
    m_categoryCombo->addItems({"Service Quality", "Guard Behavior", "Attendance Issue",
                               "Uniform Violation", "Rudeness", "Negligence",
                               "Theft/Damage", "Safety Concern", "Praise/Feedback",
                               "Other"});
    form->addRow("Category:", m_categoryCombo);

    m_sourceCombo = new QComboBox;
    m_sourceCombo->setEditable(true);
    m_sourceCombo->addItems({"Client", "Phone Call", "Email", "Written Letter",
                             "WhatsApp", "Site Visit", "Guard", "Management", "Other"});
    form->addRow("Source:", m_sourceCombo);

    m_clientCombo = new QComboBox;
    m_clientCombo->addItem("-- None --", 0);
    form->addRow("Client:", m_clientCombo);

    m_siteCombo = new QComboBox;
    m_siteCombo->addItem("-- None --", 0);
    form->addRow("Site:", m_siteCombo);

    m_guardCombo = new QComboBox;
    m_guardCombo->addItem("-- None --", 0);
    form->addRow("Against Guard:", m_guardCombo);

    m_complainantName = new QLineEdit;
    m_complainantName->setPlaceholderText("Person who complained");
    form->addRow("Complainant Name:", m_complainantName);

    m_complainantContact = new QLineEdit;
    m_complainantContact->setPlaceholderText("Phone or email");
    form->addRow("Contact:", m_complainantContact);

    m_subjectEdit = new QLineEdit;
    m_subjectEdit->setPlaceholderText("Brief subject");
    form->addRow("Subject *:", m_subjectEdit);

    m_description = new QTextEdit;
    m_description->setPlaceholderText("Detailed description...");
    m_description->setMaximumHeight(80);
    form->addRow("Description:", m_description);

    m_severityCombo = new QComboBox;
    m_severityCombo->addItems({"Low", "Medium", "High", "Critical"});
    form->addRow("Severity:", m_severityCombo);

    m_statusCombo = new QComboBox;
    m_statusCombo->addItems({"Open", "Under Review", "Resolved", "Closed", "Escalated"});
    form->addRow("Status:", m_statusCombo);

    m_assignedTo = new QLineEdit;
    m_assignedTo->setPlaceholderText("Person handling this complaint");
    form->addRow("Assigned To:", m_assignedTo);

    m_resolution = new QTextEdit;
    m_resolution->setPlaceholderText("Resolution details...");
    m_resolution->setMaximumHeight(60);
    form->addRow("Resolution:", m_resolution);

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

    auto* saveBtn = new QPushButton(m_editMode ? "Update" : "Submit Complaint");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(160, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &ComplaintDialog::saveComplaint);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void ComplaintDialog::loadClientCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT id, client_name FROM Clients WHERE status = 'Active' ORDER BY client_name");
    while (q.next()) m_clientCombo->addItem(q.value("client_name").toString(), q.value("id").toInt());
}

void ComplaintDialog::loadSiteCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT id, site_name FROM Sites WHERE status = 'Active' ORDER BY site_name");
    while (q.next()) m_siteCombo->addItem(q.value("site_name").toString(), q.value("id").toInt());
}

void ComplaintDialog::loadGuardCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT id, guard_code, full_name FROM Guards WHERE status = 'Active' ORDER BY full_name");
    while (q.next()) m_guardCombo->addItem(q.value("full_name").toString() + " (" + q.value("guard_code").toString() + ")", q.value("id").toInt());
}

void ComplaintDialog::loadComplaintData()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT * FROM Complaints WHERE id = :id", {{":id", m_complaintId}});
    if (!q.next()) return;

    m_codeEdit->setText(q.value("complaint_code").toString());
    m_typeCombo->setCurrentText(q.value("complaint_type").toString());
    m_categoryCombo->setCurrentText(q.value("category").toString());
    m_sourceCombo->setCurrentText(q.value("source").toString());

    int clientId = q.value("client_id").toInt();
    for (int i = 0; i < m_clientCombo->count(); ++i)
        if (m_clientCombo->itemData(i).toInt() == clientId) { m_clientCombo->setCurrentIndex(i); break; }
    int siteId = q.value("site_id").toInt();
    for (int i = 0; i < m_siteCombo->count(); ++i)
        if (m_siteCombo->itemData(i).toInt() == siteId) { m_siteCombo->setCurrentIndex(i); break; }
    int guardId = q.value("guard_id").toInt();
    for (int i = 0; i < m_guardCombo->count(); ++i)
        if (m_guardCombo->itemData(i).toInt() == guardId) { m_guardCombo->setCurrentIndex(i); break; }

    m_complainantName->setText(q.value("complainant_name").toString());
    m_complainantContact->setText(q.value("complainant_contact").toString());
    m_subjectEdit->setText(q.value("subject").toString());
    m_description->setPlainText(q.value("description").toString());
    m_severityCombo->setCurrentText(q.value("severity").toString());
    m_statusCombo->setCurrentText(q.value("status").toString());
    m_assignedTo->setText(q.value("assigned_to").toString());
    m_resolution->setPlainText(q.value("resolution").toString());
}

void ComplaintDialog::saveComplaint()
{
    if (m_codeEdit->text().trimmed().isEmpty()) { m_errorLabel->setText("Complaint code is required."); m_errorLabel->show(); return; }
    if (m_subjectEdit->text().trimmed().isEmpty()) { m_errorLabel->setText("Subject is required."); m_errorLabel->show(); return; }
    m_errorLabel->hide();

    auto& db = DatabaseManager::instance();
    QVariantMap data;
    data[":code"] = m_codeEdit->text().trimmed().toUpper();
    data[":type"] = m_typeCombo->currentText();
    data[":cat"] = m_categoryCombo->currentText();
    data[":source"] = m_sourceCombo->currentText();
    data[":cid"] = m_clientCombo->currentData().toInt() > 0 ? m_clientCombo->currentData() : QVariant();
    data[":sid"] = m_siteCombo->currentData().toInt() > 0 ? m_siteCombo->currentData() : QVariant();
    data[":gid"] = m_guardCombo->currentData().toInt() > 0 ? m_guardCombo->currentData() : QVariant();
    data[":cname"] = m_complainantName->text().trimmed();
    data[":ccontact"] = m_complainantContact->text().trimmed();
    data[":subject"] = m_subjectEdit->text().trimmed();
    data[":desc"] = m_description->toPlainText().trimmed();
    data[":sev"] = m_severityCombo->currentText();
    data[":status"] = m_statusCombo->currentText();
    data[":assigned"] = m_assignedTo->text().trimmed();
    data[":res"] = m_resolution->toPlainText().trimmed();

    if (m_statusCombo->currentText() == "Resolved" || m_statusCombo->currentText() == "Closed") {
        data[":rdate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
    } else {
        data[":rdate"] = QVariant();
    }

    bool ok;
    if (m_editMode) {
        data[":id"] = m_complaintId;
        ok = db.executeNonQuery("UPDATE Complaints SET complaint_code=:code, complaint_type=:type, category=:cat, source=:source, client_id=:cid, site_id=:sid, guard_id=:gid, complainant_name=:cname, complainant_contact=:ccontact, subject=:subject, description=:desc, severity=:sev, status=:status, assigned_to=:assigned, resolution=:res, resolved_date=:rdate WHERE id=:id", data);
    } else {
        ok = db.executeNonQuery("INSERT INTO Complaints (complaint_code, complaint_type, category, source, client_id, site_id, guard_id, complainant_name, complainant_contact, subject, description, severity, status, assigned_to, resolution, resolved_date) VALUES (:code, :type, :cat, :source, :cid, :sid, :gid, :cname, :ccontact, :subject, :desc, :sev, :status, :assigned, :res, :rdate)", data);
    }

    if (ok) accept();
    else { m_errorLabel->setText("Failed to save. Code may already exist."); m_errorLabel->show(); }
}
