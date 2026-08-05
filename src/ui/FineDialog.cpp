#include "FineDialog.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDate>

FineDialog::FineDialog(QWidget* parent, int fineId)
    : QDialog(parent), m_fineId(fineId), m_editMode(fineId > 0)
{
    buildUI();
    loadGuardCombo();
    if (m_editMode) loadFineData();
}

void FineDialog::buildUI()
{
    setWindowTitle(m_editMode ? "Edit Fine" : "Record Fine / Deduction");
    setMinimumSize(500, 520);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel(m_editMode ? "EDIT FINE" : "RECORD FINE / DEDUCTION");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* formGroup = new QGroupBox("Fine Details");
    auto* form = new QFormLayout(formGroup);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_guardCombo = new QComboBox;
    m_guardCombo->addItem("-- Select Guard --", 0);
    form->addRow("Guard *:", m_guardCombo);

    m_typeCombo = new QComboBox;
    m_typeCombo->setEditable(true);
    m_typeCombo->addItems({"Late Arrival", "Early Departure", "Absent Without Leave",
                           "Uniform Violation", "Sleeping on Duty", "Misconduct",
                           "Damage to Property", "Mobile Phone Usage", "Insobriety",
                           "Negligence of Duty", "Other"});
    form->addRow("Fine Type *:", m_typeCombo);

    m_reason = new QTextEdit;
    m_reason->setPlaceholderText("Detailed reason for the fine...");
    m_reason->setMaximumHeight(60);
    form->addRow("Reason *:", m_reason);

    m_amount = new QDoubleSpinBox;
    m_amount->setRange(0, 100000);
    m_amount->setDecimals(0);
    m_amount->setPrefix("Rs. ");
    m_amount->setSingleStep(100);
    form->addRow("Amount *:", m_amount);

    m_fineDate = new QDateEdit;
    m_fineDate->setCalendarPopup(true);
    m_fineDate->setDisplayFormat("yyyy-MM-dd");
    m_fineDate->setDate(QDate::currentDate());
    form->addRow("Fine Date:", m_fineDate);

    m_monthCombo = new QComboBox;
    m_monthCombo->addItems({"January", "February", "March", "April", "May", "June",
                            "July", "August", "September", "October", "November", "December"});
    m_monthCombo->setCurrentIndex(QDate::currentDate().month() - 1);
    form->addRow("Deduct in Month:", m_monthCombo);

    m_yearSpin = new QSpinBox;
    m_yearSpin->setRange(2020, 2035);
    m_yearSpin->setValue(QDate::currentDate().year());
    form->addRow("Year:", m_yearSpin);

    m_statusCombo = new QComboBox;
    m_statusCombo->addItems({"Pending", "Approved", "Deducted", "Cancelled"});
    form->addRow("Status:", m_statusCombo);

    m_approvedBy = new QLineEdit;
    m_approvedBy->setPlaceholderText("Manager/supervisor name");
    form->addRow("Approved By:", m_approvedBy);

    m_notes = new QTextEdit;
    m_notes->setPlaceholderText("Additional notes...");
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

    auto* saveBtn = new QPushButton(m_editMode ? "Update" : "Record Fine");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(140, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &FineDialog::saveFine);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void FineDialog::loadGuardCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT id, guard_code, full_name FROM Guards WHERE status = 'Active' ORDER BY full_name");
    while (q.next()) {
        m_guardCombo->addItem(q.value("full_name").toString() + " (" + q.value("guard_code").toString() + ")", q.value("id").toInt());
    }
}

void FineDialog::loadFineData()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT * FROM Fines WHERE id = :id", {{":id", m_fineId}});
    if (!q.next()) return;

    int guardId = q.value("guard_id").toInt();
    for (int i = 0; i < m_guardCombo->count(); ++i)
        if (m_guardCombo->itemData(i).toInt() == guardId) { m_guardCombo->setCurrentIndex(i); break; }

    m_typeCombo->setCurrentText(q.value("fine_type").toString());
    m_reason->setPlainText(q.value("reason").toString());
    m_amount->setValue(q.value("amount").toDouble());
    m_fineDate->setDate(QDate::fromString(q.value("fine_date").toString(), "yyyy-MM-dd"));
    m_monthCombo->setCurrentIndex(q.value("deduction_month").toInt() - 1);
    m_yearSpin->setValue(q.value("deduction_year").toInt());
    m_statusCombo->setCurrentText(q.value("status").toString());
    m_approvedBy->setText(q.value("approved_by").toString());
    m_notes->setPlainText(q.value("notes").toString());
}

void FineDialog::saveFine()
{
    if (m_guardCombo->currentData().toInt() == 0) { m_errorLabel->setText("Please select a guard."); m_errorLabel->show(); return; }
    if (m_amount->value() <= 0) { m_errorLabel->setText("Amount must be greater than zero."); m_errorLabel->show(); return; }
    m_errorLabel->hide();

    auto& db = DatabaseManager::instance();
    QVariantMap data;
    data[":gid"] = m_guardCombo->currentData();
    data[":type"] = m_typeCombo->currentText();
    data[":reason"] = m_reason->toPlainText().trimmed();
    data[":amount"] = m_amount->value();
    data[":fdate"] = m_fineDate->date().toString("yyyy-MM-dd");
    data[":month"] = m_monthCombo->currentIndex() + 1;
    data[":year"] = m_yearSpin->value();
    data[":status"] = m_statusCombo->currentText();
    data[":approved"] = m_approvedBy->text().trimmed();
    data[":notes"] = m_notes->toPlainText().trimmed();

    bool ok;
    if (m_editMode) {
        data[":id"] = m_fineId;
        ok = db.executeNonQuery("UPDATE Fines SET guard_id=:gid, fine_type=:type, reason=:reason, amount=:amount, fine_date=:fdate, deduction_month=:month, deduction_year=:year, status=:status, approved_by=:approved, notes=:notes WHERE id=:id", data);
    } else {
        ok = db.executeNonQuery("INSERT INTO Fines (guard_id, fine_type, reason, amount, fine_date, deduction_month, deduction_year, status, approved_by, notes) VALUES (:gid, :type, :reason, :amount, :fdate, :month, :year, :status, :approved, :notes)", data);
    }

    if (ok) accept();
    else { m_errorLabel->setText("Failed to save fine record."); m_errorLabel->show(); }
}
