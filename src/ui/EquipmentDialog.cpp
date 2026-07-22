#include "EquipmentDialog.h"
#include "database/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDate>

EquipmentDialog::EquipmentDialog(Mode mode, QWidget* parent, int id)
    : QDialog(parent), m_mode(mode), m_id(id)
{
    if (m_mode == ItemMode)       buildItemUI();
    else if (m_mode == IssueMode) buildIssueUI();
    else                          buildReturnUI();
}

void EquipmentDialog::buildItemUI()
{
    bool edit = m_id > 0;
    setWindowTitle(edit ? "Edit Equipment" : "Add Equipment");
    setMinimumSize(500, 500);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel(edit ? "EDIT EQUIPMENT" : "ADD EQUIPMENT");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* formGroup = new QGroupBox("Equipment Details");
    auto* form = new QFormLayout(formGroup);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_codeEdit = new QLineEdit;
    m_codeEdit->setPlaceholderText("e.g. EQ-001");
    form->addRow("Equipment Code *:", m_codeEdit);

    m_typeEdit = new QLineEdit;
    m_typeEdit->setPlaceholderText("e.g. Walkie Talkie, Torch, Baton...");
    form->addRow("Type *:", m_typeEdit);

    m_descEdit = new QTextEdit;
    m_descEdit->setPlaceholderText("Description...");
    m_descEdit->setMaximumHeight(60);
    form->addRow("Description:", m_descEdit);

    m_serialEdit = new QLineEdit;
    m_serialEdit->setPlaceholderText("Serial number...");
    form->addRow("Serial Number:", m_serialEdit);

    m_purchaseDate = new QDateEdit;
    m_purchaseDate->setCalendarPopup(true);
    m_purchaseDate->setDisplayFormat("yyyy-MM-dd");
    m_purchaseDate->setDate(QDate::currentDate());
    m_purchaseDate->setSpecialValueText("Not set");
    form->addRow("Purchase Date:", m_purchaseDate);

    m_conditionCombo = new QComboBox;
    m_conditionCombo->addItems({"New", "Good", "Fair", "Poor", "Damaged"});
    form->addRow("Condition:", m_conditionCombo);

    m_statusCombo = new QComboBox;
    m_statusCombo->addItems({"Available", "Issued", "Under Repair", "Retired", "Lost"});
    form->addRow("Status:", m_statusCombo);

    m_notes = new QTextEdit;
    m_notes->setPlaceholderText("Notes...");
    m_notes->setMaximumHeight(60);
    form->addRow("Notes:", m_notes);

    mainLayout->addWidget(formGroup);
    mainLayout->addStretch();

    if (edit) loadItemData();

    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("SecondaryButton");
    cancelBtn->setFixedSize(100, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);

    auto* saveBtn = new QPushButton(edit ? "Update" : "Add Equipment");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(150, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &EquipmentDialog::save);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void EquipmentDialog::buildIssueUI()
{
    setWindowTitle("Issue Equipment");
    setMinimumSize(480, 420);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("ISSUE EQUIPMENT TO GUARD");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* formGroup = new QGroupBox("Issue Details");
    auto* form = new QFormLayout(formGroup);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_guardCombo = new QComboBox;
    m_guardCombo->addItem("-- Select Guard --", 0);
    loadGuardCombo();
    form->addRow("Guard *:", m_guardCombo);

    m_equipCombo = new QComboBox;
    m_equipCombo->addItem("-- Select Equipment --", 0);
    loadEquipmentCombo();
    form->addRow("Equipment *:", m_equipCombo);

    m_conditionOut = new QComboBox;
    m_conditionOut->addItems({"New", "Good", "Fair", "Poor"});
    form->addRow("Condition at Issue:", m_conditionOut);

    m_notes = new QTextEdit;
    m_notes->setPlaceholderText("Notes...");
    m_notes->setMaximumHeight(60);
    form->addRow("Notes:", m_notes);

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

    auto* saveBtn = new QPushButton("Issue Equipment");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(160, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &EquipmentDialog::save);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void EquipmentDialog::buildReturnUI()
{
    setWindowTitle("Return Equipment");
    setMinimumSize(450, 350);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("RETURN EQUIPMENT");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* formGroup = new QGroupBox("Return Details");
    auto* form = new QFormLayout(formGroup);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_conditionIn = new QComboBox;
    m_conditionIn->addItems({"Good", "Fair", "Poor", "Damaged"});
    form->addRow("Return Condition *:", m_conditionIn);

    m_returnNotes = new QTextEdit;
    m_returnNotes->setPlaceholderText("Return notes...");
    m_returnNotes->setMaximumHeight(80);
    form->addRow("Notes:", m_returnNotes);

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

    auto* saveBtn = new QPushButton("Confirm Return");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(150, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &EquipmentDialog::save);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void EquipmentDialog::loadItemData()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT * FROM Equipment WHERE id = :id", {{":id", m_id}});
    if (!q.next()) return;

    m_codeEdit->setText(q.value("equipment_code").toString());
    m_typeEdit->setText(q.value("equipment_type").toString());
    m_descEdit->setPlainText(q.value("description").toString());
    m_serialEdit->setText(q.value("serial_number").toString());

    QDate pdate = QDate::fromString(q.value("purchase_date").toString(), "yyyy-MM-dd");
    if (pdate.isValid()) m_purchaseDate->setDate(pdate);

    m_conditionCombo->setCurrentText(q.value("condition").toString());
    m_statusCombo->setCurrentText(q.value("status").toString());
    m_notes->setPlainText(q.value("notes").toString());
}

void EquipmentDialog::loadGuardCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute(
        "SELECT id, guard_code, full_name FROM Guards "
        "WHERE status = 'Active' ORDER BY full_name"
    );
    while (q.next()) {
        m_guardCombo->addItem(
            q.value("full_name").toString() + " (" + q.value("guard_code").toString() + ")",
            q.value("id").toInt()
        );
    }
}

void EquipmentDialog::loadEquipmentCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute(
        "SELECT id, equipment_code, equipment_type, condition FROM Equipment "
        "WHERE status = 'Available' ORDER BY equipment_code"
    );
    while (q.next()) {
        m_equipCombo->addItem(
            q.value("equipment_code").toString() + " - " + q.value("equipment_type").toString()
            + " [" + q.value("condition").toString() + "]",
            q.value("id").toInt()
        );
    }
}

void EquipmentDialog::save()
{
    auto& db = DatabaseManager::instance();

    if (m_mode == ItemMode) {
        if (m_codeEdit->text().trimmed().isEmpty()) {
            m_errorLabel->setText("Equipment code is required."); m_errorLabel->show(); return;
        }
        if (m_typeEdit->text().trimmed().isEmpty()) {
            m_errorLabel->setText("Equipment type is required."); m_errorLabel->show(); return;
        }

        QVariantMap data;
        data[":code"]  = m_codeEdit->text().trimmed();
        data[":type"]  = m_typeEdit->text().trimmed();
        data[":desc"]  = m_descEdit->toPlainText().trimmed();
        data[":serial"] = m_serialEdit->text().trimmed();
        data[":date"]  = m_purchaseDate->date().toString("yyyy-MM-dd");
        data[":cond"]  = m_conditionCombo->currentText();
        data[":status"] = m_statusCombo->currentText();
        data[":notes"] = m_notes->toPlainText().trimmed();

        bool ok;
        if (m_id > 0) {
            data[":id"] = m_id;
            ok = db.executeNonQuery(
                "UPDATE Equipment SET equipment_code = :code, equipment_type = :type, "
                "description = :desc, serial_number = :serial, purchase_date = :date, "
                "condition = :cond, status = :status, notes = :notes WHERE id = :id", data
            );
        } else {
            ok = db.executeNonQuery(
                "INSERT INTO Equipment (equipment_code, equipment_type, description, "
                "serial_number, purchase_date, condition, status, notes) "
                "VALUES (:code, :type, :desc, :serial, :date, :cond, :status, :notes)", data
            );
        }

        if (ok) accept();
        else { m_errorLabel->setText("Failed to save. Code may already exist."); m_errorLabel->show(); }

    } else if (m_mode == IssueMode) {
        if (m_guardCombo->currentData().toInt() == 0) {
            m_errorLabel->setText("Please select a guard."); m_errorLabel->show(); return;
        }
        if (m_equipCombo->currentData().toInt() == 0) {
            m_errorLabel->setText("Please select equipment."); m_errorLabel->show(); return;
        }

        int equipId = m_equipCombo->currentData().toInt();

        QVariantMap data;
        data[":eid"]  = equipId;
        data[":gid"]  = m_guardCombo->currentData().toInt();
        data[":date"] = QDate::currentDate().toString("yyyy-MM-dd");
        data[":cond"] = m_conditionOut->currentText();
        data[":notes"] = m_notes->toPlainText().trimmed();

        bool ok = db.executeNonQuery(
            "INSERT INTO EquipmentIssue (equipment_id, guard_id, issue_date, condition_out, notes) "
            "VALUES (:eid, :gid, :date, :cond, :notes)", data
        );

        if (ok) {
            db.executeNonQuery(
                "UPDATE Equipment SET status = 'Issued' WHERE id = :id", {{":id", equipId}}
            );
            accept();
        } else {
            m_errorLabel->setText("Failed to issue equipment."); m_errorLabel->show();
        }

    } else {
        // Return mode
        QVariantMap data;
        data[":id"]    = m_id;
        data[":date"]  = QDate::currentDate().toString("yyyy-MM-dd");
        data[":cond"]  = m_conditionIn->currentText();
        data[":notes"] = m_returnNotes ? m_returnNotes->toPlainText().trimmed() : "";

        bool ok = db.executeNonQuery(
            "UPDATE EquipmentIssue SET return_date = :date, condition_in = :cond, "
            "notes = CASE WHEN notes = '' THEN :notes ELSE notes || ' | ' || :notes END "
            "WHERE id = :id", data
        );

        if (ok) {
            auto uq = db.execute("SELECT equipment_id FROM EquipmentIssue WHERE id = :id", {{":id", m_id}});
            if (uq.next()) {
                int eid = uq.value("equipment_id").toInt();
                QString newCond = m_conditionIn->currentText();
                db.executeNonQuery(
                    "UPDATE Equipment SET status = 'Available', condition = :cond WHERE id = :id",
                    {{":cond", newCond}, {":id", eid}}
                );
            }
            accept();
        } else {
            m_errorLabel->setText("Failed to process return."); m_errorLabel->show();
        }
    }
}
