#include "UniformDialog.h"
#include "database/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDate>

UniformDialog::UniformDialog(Mode mode, QWidget* parent, int id)
    : QDialog(parent), m_mode(mode), m_id(id)
{
    if (m_mode == ItemMode)      buildItemUI();
    else if (m_mode == IssueMode) buildIssueUI();
    else                          buildReturnUI();
}

int UniformDialog::selectedGuardId() const
{
    return m_guardCombo ? m_guardCombo->currentData().toInt() : 0;
}

int UniformDialog::selectedUniformId() const
{
    return m_uniformCombo ? m_uniformCombo->currentData().toInt() : 0;
}

void UniformDialog::buildItemUI()
{
    bool edit = m_id > 0;
    setWindowTitle(edit ? "Edit Uniform Item" : "Add Uniform Item");
    setMinimumSize(450, 380);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel(edit ? "EDIT UNIFORM ITEM" : "ADD UNIFORM ITEM");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* formGroup = new QGroupBox("Item Details");
    auto* form = new QFormLayout(formGroup);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_itemType = new QLineEdit;
    m_itemType->setPlaceholderText("e.g. Full Dress Shirt");
    form->addRow("Item Type *:", m_itemType);

    m_sizeCombo = new QComboBox;
    m_sizeCombo->setEditable(true);
    m_sizeCombo->addItems({"S", "M", "L", "XL", "XXL", "Free Size"});
    form->addRow("Size:", m_sizeCombo);

    m_qtySpin = new QSpinBox;
    m_qtySpin->setRange(0, 99999);
    m_qtySpin->setValue(1);
    form->addRow("Quantity:", m_qtySpin);

    m_statusCombo = new QComboBox;
    m_statusCombo->addItems({"Available", "Issued", "Damaged", "Retired"});
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

    auto* saveBtn = new QPushButton(edit ? "Update" : "Add Item");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(110, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &UniformDialog::save);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void UniformDialog::buildIssueUI()
{
    setWindowTitle("Issue Uniform");
    setMinimumSize(480, 420);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("ISSUE UNIFORM TO GUARD");
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

    m_uniformCombo = new QComboBox;
    m_uniformCombo->addItem("-- Select Uniform --", 0);
    loadUniformCombo();
    form->addRow("Uniform Item *:", m_uniformCombo);

    m_conditionOut = new QComboBox;
    m_conditionOut->addItems({"New", "Good", "Worn"});
    form->addRow("Condition:", m_conditionOut);

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

    auto* saveBtn = new QPushButton("Issue Uniform");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(140, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &UniformDialog::save);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void UniformDialog::buildReturnUI()
{
    setWindowTitle("Return Uniform");
    setMinimumSize(450, 350);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("RETURN UNIFORM");
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
    m_conditionIn->addItems({"Good", "Worn", "Damaged"});
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
    saveBtn->setFixedSize(140, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &UniformDialog::save);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void UniformDialog::loadItemData()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT * FROM Uniform WHERE id = :id", {{":id", m_id}});
    if (!q.next()) return;

    m_itemType->setText(q.value("item_type").toString());
    m_sizeCombo->setCurrentText(q.value("size").toString());
    m_qtySpin->setValue(q.value("quantity").toInt());
    m_statusCombo->setCurrentText(q.value("status").toString());
    m_notes->setPlainText(q.value("notes").toString());
}

void UniformDialog::loadGuardCombo()
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

void UniformDialog::loadUniformCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute(
        "SELECT id, item_type, size, quantity FROM Uniform "
        "WHERE status = 'Available' AND quantity > 0 ORDER BY item_type"
    );
    while (q.next()) {
        m_uniformCombo->addItem(
            q.value("item_type").toString() + " [Size: " + q.value("size").toString()
            + "] (Qty: " + q.value("quantity").toString() + ")",
            q.value("id").toInt()
        );
    }
}

void UniformDialog::save()
{
    auto& db = DatabaseManager::instance();

    if (m_mode == ItemMode) {
        if (m_itemType->text().trimmed().isEmpty()) {
            m_errorLabel->setText("Item type is required.");
            m_errorLabel->show(); return;
        }

        QVariantMap data;
        data[":type"]   = m_itemType->text().trimmed();
        data[":size"]   = m_sizeCombo->currentText();
        data[":qty"]    = m_qtySpin->value();
        data[":status"] = m_statusCombo->currentText();
        data[":notes"]  = m_notes->toPlainText().trimmed();

        bool ok;
        if (m_id > 0) {
            data[":id"] = m_id;
            ok = db.executeNonQuery(
                "UPDATE Uniform SET item_type = :type, size = :size, quantity = :qty, "
                "status = :status, notes = :notes WHERE id = :id", data
            );
        } else {
            ok = db.executeNonQuery(
                "INSERT INTO Uniform (item_type, size, quantity, status, notes) "
                "VALUES (:type, :size, :qty, :status, :notes)", data
            );
        }

        if (ok) accept();
        else { m_errorLabel->setText("Failed to save uniform item."); m_errorLabel->show(); }

    } else if (m_mode == IssueMode) {
        if (m_guardCombo->currentData().toInt() == 0) {
            m_errorLabel->setText("Please select a guard.");
            m_errorLabel->show(); return;
        }
        if (m_uniformCombo->currentData().toInt() == 0) {
            m_errorLabel->setText("Please select a uniform item.");
            m_errorLabel->show(); return;
        }

        int uniformId = m_uniformCombo->currentData().toInt();

        // Check stock
        auto sq = db.execute("SELECT quantity, item_type FROM Uniform WHERE id = :id", {{":id", uniformId}});
        if (!sq.next() || sq.value("quantity").toInt() <= 0) {
            m_errorLabel->setText("Selected uniform is out of stock.");
            m_errorLabel->show(); return;
        }

        // Insert issuance record
        QVariantMap data;
        data[":uid"]   = uniformId;
        data[":gid"]   = m_guardCombo->currentData().toInt();
        data[":date"]  = QDate::currentDate().toString("yyyy-MM-dd");
        data[":cond"]  = m_conditionOut->currentText();
        data[":notes"] = m_notes->toPlainText().trimmed();

        bool ok = db.executeNonQuery(
            "INSERT INTO UniformIssue (uniform_id, guard_id, issue_date, condition_out, notes) "
            "VALUES (:uid, :gid, :date, :cond, :notes)", data
        );

        if (ok) {
            // Decrease quantity
            db.executeNonQuery(
                "UPDATE Uniform SET quantity = quantity - 1 WHERE id = :id AND quantity > 0",
                {{":id", uniformId}}
            );
            // Update status if stock is now 0
            db.executeNonQuery(
                "UPDATE Uniform SET status = 'Issued' WHERE id = :id AND quantity <= 0",
                {{":id", uniformId}}
            );
            accept();
        } else {
            m_errorLabel->setText("Failed to issue uniform.");
            m_errorLabel->show();
        }

    } else {
        // Return mode
        if (!m_conditionIn) {
            m_errorLabel->setText("Invalid return dialog state.");
            m_errorLabel->show(); return;
        }

        QVariantMap data;
        data[":id"]    = m_id;
        data[":date"]  = QDate::currentDate().toString("yyyy-MM-dd");
        data[":cond"]  = m_conditionIn->currentText();
        data[":notes"] = m_returnNotes ? m_returnNotes->toPlainText().trimmed() : "";

        bool ok = db.executeNonQuery(
            "UPDATE UniformIssue SET return_date = :date, condition_in = :cond, "
            "notes = CASE WHEN notes = '' THEN :notes ELSE notes || ' | ' || :notes END "
            "WHERE id = :id", data
        );

        if (ok) {
            // Get uniform_id to increase stock back
            auto uq = db.execute("SELECT uniform_id FROM UniformIssue WHERE id = :id", {{":id", m_id}});
            if (uq.next()) {
                int uid = uq.value("uniform_id").toInt();
                db.executeNonQuery(
                    "UPDATE Uniform SET quantity = quantity + 1, status = 'Available' WHERE id = :id",
                    {{":id", uid}}
                );
            }
            accept();
        } else {
            m_errorLabel->setText("Failed to process return.");
            m_errorLabel->show();
        }
    }
}
