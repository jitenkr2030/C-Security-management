#include "TicketDialog.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDate>

TicketDialog::TicketDialog(QWidget* parent, int ticketId)
    : QDialog(parent), m_ticketId(ticketId), m_editMode(ticketId > 0)
{
    buildUI();
    if (m_editMode) loadTicketData();
}

void TicketDialog::buildUI()
{
    setWindowTitle(m_editMode ? "Edit Ticket" : "New Helpdesk Ticket");
    setMinimumSize(500, 500);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel(m_editMode ? "EDIT TICKET" : "NEW HELPDESK TICKET");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* formGroup = new QGroupBox("Ticket Details");
    auto* form = new QFormLayout(formGroup);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_codeEdit = new QLineEdit;
    m_codeEdit->setPlaceholderText("e.g. TKT-001");
    form->addRow("Ticket Code *:", m_codeEdit);

    m_categoryCombo = new QComboBox;
    m_categoryCombo->setEditable(true);
    m_categoryCombo->addItems({"IT Issue", "Hardware", "Software", "Network",
                               "Salary Issue", "Leave Issue", "Uniform Issue",
                               "Equipment Issue", "Complaint", "Suggestion",
                               "General", "Other"});
    form->addRow("Category:", m_categoryCombo);

    m_priorityCombo = new QComboBox;
    m_priorityCombo->addItems({"Low", "Medium", "High", "Critical"});
    form->addRow("Priority:", m_priorityCombo);

    m_subjectEdit = new QLineEdit;
    m_subjectEdit->setPlaceholderText("Brief subject");
    form->addRow("Subject *:", m_subjectEdit);

    m_description = new QTextEdit;
    m_description->setPlaceholderText("Detailed description of the issue...");
    m_description->setMaximumHeight(80);
    form->addRow("Description:", m_description);

    m_raisedBy = new QLineEdit;
    m_raisedBy->setPlaceholderText("Person who raised this ticket");
    form->addRow("Raised By:", m_raisedBy);

    m_assignedTo = new QLineEdit;
    m_assignedTo->setPlaceholderText("Person assigned to resolve");
    form->addRow("Assigned To:", m_assignedTo);

    m_statusCombo = new QComboBox;
    m_statusCombo->addItems({"Open", "In Progress", "Resolved", "Closed", "On Hold"});
    form->addRow("Status:", m_statusCombo);

    m_resolution = new QTextEdit;
    m_resolution->setPlaceholderText("Resolution details...");
    m_resolution->setMaximumHeight(60);
    form->addRow("Resolution:", m_resolution);

    mainLayout->addWidget(formGroup, 1);

    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("SecondaryButton");
    cancelBtn->setFixedSize(100, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);
    auto* saveBtn = new QPushButton(m_editMode ? "Update" : "Create Ticket");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(140, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &TicketDialog::saveTicket);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void TicketDialog::loadTicketData()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT * FROM Tickets WHERE id = :id", {{":id", m_ticketId}});
    if (!q.next()) return;

    m_codeEdit->setText(q.value("ticket_code").toString());
    m_categoryCombo->setCurrentText(q.value("category").toString());
    m_priorityCombo->setCurrentText(q.value("priority").toString());
    m_subjectEdit->setText(q.value("subject").toString());
    m_description->setPlainText(q.value("description").toString());
    m_raisedBy->setText(q.value("raised_by").toString());
    m_assignedTo->setText(q.value("assigned_to").toString());
    m_statusCombo->setCurrentText(q.value("status").toString());
    m_resolution->setPlainText(q.value("resolution").toString());
}

void TicketDialog::saveTicket()
{
    if (m_codeEdit->text().trimmed().isEmpty()) { m_errorLabel->setText("Ticket code is required."); m_errorLabel->show(); return; }
    if (m_subjectEdit->text().trimmed().isEmpty()) { m_errorLabel->setText("Subject is required."); m_errorLabel->show(); return; }
    m_errorLabel->hide();

    auto& db = DatabaseManager::instance();
    QVariantMap data;
    data[":code"] = m_codeEdit->text().trimmed().toUpper();
    data[":cat"] = m_categoryCombo->currentText();
    data[":pri"] = m_priorityCombo->currentText();
    data[":subject"] = m_subjectEdit->text().trimmed();
    data[":desc"] = m_description->toPlainText().trimmed();
    data[":raised"] = m_raisedBy->text().trimmed();
    data[":assigned"] = m_assignedTo->text().trimmed();
    data[":status"] = m_statusCombo->currentText();
    data[":res"] = m_resolution->toPlainText().trimmed();
    data[":rdate"] = (m_statusCombo->currentText() == "Resolved" || m_statusCombo->currentText() == "Closed")
        ? QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm") : QVariant();

    bool ok;
    if (m_editMode) {
        data[":id"] = m_ticketId;
        ok = db.executeNonQuery("UPDATE Tickets SET ticket_code=:code, category=:cat, priority=:pri, subject=:subject, description=:desc, raised_by=:raised, assigned_to=:assigned, status=:status, resolution=:res, resolved_date=:rdate WHERE id=:id", data);
    } else {
        ok = db.executeNonQuery("INSERT INTO Tickets (ticket_code, category, priority, subject, description, raised_by, assigned_to, status, resolution, resolved_date) VALUES (:code, :cat, :pri, :subject, :desc, :raised, :assigned, :status, :res, :rdate)", data);
    }

    if (ok) accept();
    else { m_errorLabel->setText("Failed to save. Code may already exist."); m_errorLabel->show(); }
}
