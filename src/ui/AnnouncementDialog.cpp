#include "AnnouncementDialog.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDate>

AnnouncementDialog::AnnouncementDialog(QWidget* parent, int announcementId)
    : QDialog(parent), m_announcementId(announcementId), m_editMode(announcementId > 0)
{
    buildUI();
    loadSiteCombo();
    if (m_editMode) loadAnnouncementData();
}

void AnnouncementDialog::buildUI()
{
    setWindowTitle(m_editMode ? "Edit Announcement" : "New Announcement / Circular");
    setMinimumSize(520, 520);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel(m_editMode ? "EDIT ANNOUNCEMENT" : "NEW ANNOUNCEMENT / CIRCULAR");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* formGroup = new QGroupBox("Announcement Details");
    auto* form = new QFormLayout(formGroup);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_titleEdit = new QLineEdit;
    m_titleEdit->setPlaceholderText("Brief title for the announcement");
    form->addRow("Title *:", m_titleEdit);

    m_message = new QTextEdit;
    m_message->setPlaceholderText("Full message content...");
    m_message->setMaximumHeight(120);
    form->addRow("Message *:", m_message);

    m_targetType = new QComboBox;
    m_targetType->addItems({"All", "Specific Site", "Specific Guard"});
    connect(m_targetType, &QComboBox::currentTextChanged, this, [this](const QString& text) {
        m_targetCombo->setVisible(text != "All");
        if (text == "All") m_targetCombo->setCurrentIndex(0);
    });
    form->addRow("Target:", m_targetType);

    m_targetCombo = new QComboBox;
    m_targetCombo->addItem("-- Select --", 0);
    m_targetCombo->setVisible(false);
    form->addRow("Target Detail:", m_targetCombo);

    m_priorityCombo = new QComboBox;
    m_priorityCombo->addItems({"Normal", "Important", "Urgent"});
    form->addRow("Priority:", m_priorityCombo);

    m_publishedBy = new QLineEdit;
    m_publishedBy->setPlaceholderText("Your name");
    form->addRow("Published By:", m_publishedBy);

    m_expiresAt = new QDateTimeEdit;
    m_expiresAt->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_expiresAt->setDateTime(QDateTime::currentDateTime().addDays(30));
    m_expiresAt->setCalendarPopup(true);
    form->addRow("Expires On:", m_expiresAt);

    m_statusCombo = new QComboBox;
    m_statusCombo->addItems({"Active", "Draft", "Expired"});
    form->addRow("Status:", m_statusCombo);

    mainLayout->addWidget(formGroup, 1);

    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("SecondaryButton");
    cancelBtn->setFixedSize(100, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);
    auto* saveBtn = new QPushButton(m_editMode ? "Update" : "Publish");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(120, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &AnnouncementDialog::saveAnnouncement);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void AnnouncementDialog::loadSiteCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT id, site_name FROM Sites WHERE status = 'Active' ORDER BY site_name");
    while (q.next()) m_targetCombo->addItem(q.value("site_name").toString(), q.value("id").toInt());
}

void AnnouncementDialog::loadAnnouncementData()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT * FROM Announcements WHERE id = :id", {{":id", m_announcementId}});
    if (!q.next()) return;

    m_titleEdit->setText(q.value("title").toString());
    m_message->setPlainText(q.value("message").toString());
    m_targetType->setCurrentText(q.value("target_type").toString());

    int targetId = q.value("target_id").toInt();
    if (targetId > 0) {
        for (int i = 0; i < m_targetCombo->count(); ++i)
            if (m_targetCombo->itemData(i).toInt() == targetId) { m_targetCombo->setCurrentIndex(i); break; }
    }

    m_priorityCombo->setCurrentText(q.value("priority").toString());
    m_publishedBy->setText(q.value("published_by").toString());

    QDateTime expires = QDateTime::fromString(q.value("expires_at").toString(), "yyyy-MM-dd HH:mm");
    if (expires.isValid()) m_expiresAt->setDateTime(expires);

    m_statusCombo->setCurrentText(q.value("status").toString());
}

void AnnouncementDialog::saveAnnouncement()
{
    if (m_titleEdit->text().trimmed().isEmpty()) { m_errorLabel->setText("Title is required."); m_errorLabel->show(); return; }
    if (m_message->toPlainText().trimmed().isEmpty()) { m_errorLabel->setText("Message is required."); m_errorLabel->show(); return; }
    m_errorLabel->hide();

    auto& db = DatabaseManager::instance();
    QVariantMap data;
    data[":title"] = m_titleEdit->text().trimmed();
    data[":msg"] = m_message->toPlainText().trimmed();
    data[":targetType"] = m_targetType->currentText();
    data[":targetId"] = m_targetCombo->currentData().toInt() > 0 ? m_targetCombo->currentData() : 0;
    data[":priority"] = m_priorityCombo->currentText();
    data[":by"] = m_publishedBy->text().trimmed();
    data[":expires"] = m_expiresAt->dateTime().toString("yyyy-MM-dd HH:mm");
    data[":status"] = m_statusCombo->currentText();

    bool ok;
    if (m_editMode) {
        data[":id"] = m_announcementId;
        ok = db.executeNonQuery("UPDATE Announcements SET title=:title, message=:msg, target_type=:targetType, target_id=:targetId, priority=:priority, published_by=:by, expires_at=:expires, status=:status WHERE id=:id", data);
    } else {
        ok = db.executeNonQuery("INSERT INTO Announcements (title, message, target_type, target_id, priority, published_by, expires_at, status) VALUES (:title, :msg, :targetType, :targetId, :priority, :by, :expires, :status)", data);
    }

    if (ok) accept();
    else { m_errorLabel->setText("Failed to save announcement."); m_errorLabel->show(); }
}
