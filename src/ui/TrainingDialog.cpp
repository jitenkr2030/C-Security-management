#include "TrainingDialog.h"
#include <QHeaderView>
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>

TrainingDialog::TrainingDialog(QWidget* parent, int trainingId)
    : QDialog(parent), m_trainingId(trainingId), m_editMode(trainingId > 0), m_guardCombo(nullptr), m_partTable(nullptr), m_partCount(nullptr)
{
    buildUI();
    if (m_editMode) { loadTrainingData(); loadParticipants(); }
}

void TrainingDialog::buildUI()
{
    setWindowTitle(m_editMode ? "Edit Training" : "New Training");
    setMinimumSize(600, 650);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(12);

    auto* titleLabel = new QLabel(m_editMode ? "EDIT TRAINING" : "NEW TRAINING PROGRAM");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #E85454; font-weight: 600; font-size: 12px;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto* formGroup = new QGroupBox("Training Details");
    auto* form = new QFormLayout(formGroup);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText("e.g. Fire Safety Training");
    form->addRow("Training Name *:", m_nameEdit);

    m_typeCombo = new QComboBox;
    m_typeCombo->setEditable(true);
    m_typeCombo->addItems({"Fire Safety", "First Aid", "Physical Training", "Drill", "Weapon Handling", "CCTV Operations", "Crowd Management", "Legal Awareness", "Customer Service", "Self Defense", "Other"});
    form->addRow("Type:", m_typeCombo);

    m_descEdit = new QTextEdit;
    m_descEdit->setPlaceholderText("Description...");
    m_descEdit->setMaximumHeight(50);
    form->addRow("Description:", m_descEdit);

    m_trainerEdit = new QLineEdit;
    m_trainerEdit->setPlaceholderText("Trainer name");
    form->addRow("Trainer:", m_trainerEdit);

    m_startDate = new QDateEdit;
    m_startDate->setCalendarPopup(true);
    m_startDate->setDisplayFormat("yyyy-MM-dd");
    m_startDate->setDate(QDate::currentDate());
    form->addRow("Start Date *:", m_startDate);

    m_endDate = new QDateEdit;
    m_endDate->setCalendarPopup(true);
    m_endDate->setDisplayFormat("yyyy-MM-dd");
    m_endDate->setDate(QDate::currentDate().addDays(1));
    form->addRow("End Date:", m_endDate);

    m_locationEdit = new QLineEdit;
    m_locationEdit->setPlaceholderText("Training location");
    form->addRow("Location:", m_locationEdit);

    m_notes = new QTextEdit;
    m_notes->setPlaceholderText("Notes...");
    m_notes->setMaximumHeight(40);
    form->addRow("Notes:", m_notes);

    mainLayout->addWidget(formGroup);

    if (m_editMode) {
        auto* partGroup = new QGroupBox("Participants");
        auto* partLayout = new QVBoxLayout(partGroup);

        auto* addRow = new QHBoxLayout;
        m_guardCombo = new QComboBox;
        m_guardCombo->addItem("-- Select Guard --", 0);
        loadGuardCombo();
        addRow->addWidget(m_guardCombo, 1);

        auto* addPartBtn = new QPushButton("+ Add");
        addPartBtn->setObjectName("PrimaryButton");
        addPartBtn->setFixedSize(80, 32);
        addPartBtn->setCursor(Qt::PointingHandCursor);
        connect(addPartBtn, &QPushButton::clicked, this, &TrainingDialog::addParticipant);
        addRow->addWidget(addPartBtn);

        auto* remPartBtn = new QPushButton("Remove");
        remPartBtn->setObjectName("DangerButton");
        remPartBtn->setFixedSize(80, 32);
        remPartBtn->setCursor(Qt::PointingHandCursor);
        connect(remPartBtn, &QPushButton::clicked, this, &TrainingDialog::removeParticipant);
        addRow->addWidget(remPartBtn);

        m_partCount = new QLabel;
        m_partCount->setStyleSheet("color: #8B95A5; font-weight: 600;");
        addRow->addWidget(m_partCount);
        partLayout->addLayout(addRow);

        m_partTable = new QTableWidget;
        m_partTable->setAlternatingRowColors(true);
        m_partTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_partTable->setSelectionMode(QAbstractItemView::SingleSelection);
        m_partTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_partTable->verticalHeader()->setVisible(false);
        m_partTable->setShowGrid(false);
        m_partTable->setColumnCount(5);
        m_partTable->setHorizontalHeaderLabels({"ID", "Guard ID", "Code", "Name", "Attendance"});
        m_partTable->setColumnHidden(0, true);
        m_partTable->setColumnHidden(1, true);
        m_partTable->setColumnWidth(2, 100);
        m_partTable->setColumnWidth(3, 200);
        m_partTable->setColumnWidth(4, 120);
        m_partTable->horizontalHeader()->setStretchLastSection(true);
        partLayout->addWidget(m_partTable);

        mainLayout->addWidget(partGroup, 1);
    }

    mainLayout->addStretch();

    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("SecondaryButton");
    cancelBtn->setFixedSize(100, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);
    auto* saveBtn = new QPushButton(m_editMode ? "Update" : "Create Training");
    saveBtn->setObjectName("PrimaryButton");
    saveBtn->setFixedSize(140, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &TrainingDialog::saveTraining);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void TrainingDialog::loadGuardCombo()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT id, guard_code, full_name FROM Guards WHERE status = 'Active' ORDER BY full_name");
    while (q.next()) m_guardCombo->addItem(q.value("full_name").toString() + " (" + q.value("guard_code").toString() + ")", q.value("id").toInt());
}

void TrainingDialog::loadTrainingData()
{
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT * FROM Training WHERE id = :id", {{":id", m_trainingId}});
    if (!q.next()) return;
    m_nameEdit->setText(q.value("training_name").toString());
    m_typeCombo->setCurrentText(q.value("training_type").toString());
    m_descEdit->setPlainText(q.value("description").toString());
    m_trainerEdit->setText(q.value("trainer_name").toString());
    m_startDate->setDate(QDate::fromString(q.value("start_date").toString(), "yyyy-MM-dd"));
    QDate end = QDate::fromString(q.value("end_date").toString(), "yyyy-MM-dd");
    if (end.isValid()) m_endDate->setDate(end);
    m_locationEdit->setText(q.value("location").toString());
    m_notes->setPlainText(q.value("notes").toString());
}

void TrainingDialog::loadParticipants()
{
    if (!m_partTable) return;
    auto& db = DatabaseManager::instance();
    auto q = db.execute("SELECT tp.id, tp.guard_id, g.guard_code, g.full_name, tp.attendance FROM TrainingParticipant tp JOIN Guards g ON tp.guard_id = g.id WHERE tp.training_id = :tid ORDER BY g.full_name", {{":tid", m_trainingId}});
    int row = 0;
    m_partTable->setRowCount(0);
    while (q.next()) {
        m_partTable->insertRow(row);
        m_partTable->setItem(row, 0, new QTableWidgetItem(q.value("id").toString()));
        m_partTable->setItem(row, 1, new QTableWidgetItem(q.value("guard_id").toString()));
        m_partTable->setItem(row, 2, new QTableWidgetItem(q.value("guard_code").toString()));
        m_partTable->setItem(row, 3, new QTableWidgetItem(q.value("full_name").toString()));
        m_partTable->setItem(row, 4, new QTableWidgetItem(q.value("attendance").toString()));
        row++;
    }
    if (m_partCount) m_partCount->setText(QString("%1 participants").arg(row));
}

void TrainingDialog::saveTraining()
{
    if (m_nameEdit->text().trimmed().isEmpty()) { m_errorLabel->setText("Training name is required."); m_errorLabel->show(); return; }
    m_errorLabel->hide();

    auto& db = DatabaseManager::instance();
    QVariantMap data;
    data[":name"] = m_nameEdit->text().trimmed();
    data[":type"] = m_typeCombo->currentText();
    data[":desc"] = m_descEdit->toPlainText().trimmed();
    data[":trainer"] = m_trainerEdit->text().trimmed();
    data[":start"] = m_startDate->date().toString("yyyy-MM-dd");
    data[":end"] = m_endDate->date().toString("yyyy-MM-dd");
    data[":loc"] = m_locationEdit->text().trimmed();
    data[":notes"] = m_notes->toPlainText().trimmed();

    bool ok;
    if (m_editMode) {
        data[":id"] = m_trainingId;
        ok = db.executeNonQuery("UPDATE Training SET training_name=:name, training_type=:type, description=:desc, trainer_name=:trainer, start_date=:start, end_date=:end, location=:loc, notes=:notes WHERE id=:id", data);
    } else {
        ok = db.executeNonQuery("INSERT INTO Training (training_name, training_type, description, trainer_name, start_date, end_date, location, notes) VALUES (:name, :type, :desc, :trainer, :start, :end, :loc, :notes)", data);
    }
    if (ok) accept();
    else { m_errorLabel->setText("Failed to save training."); m_errorLabel->show(); }
}

void TrainingDialog::addParticipant()
{
    if (!m_guardCombo || m_guardCombo->currentData().toInt() == 0) { m_errorLabel->setText("Please select a guard."); m_errorLabel->show(); return; }
    m_errorLabel->hide();
    int guardId = m_guardCombo->currentData().toInt();
    for (int r = 0; r < m_partTable->rowCount(); ++r) {
        if (m_partTable->item(r, 1)->text().toInt() == guardId) { m_errorLabel->setText("Guard is already a participant."); m_errorLabel->show(); return; }
    }
    auto& db = DatabaseManager::instance();
    db.executeNonQuery("INSERT INTO TrainingParticipant (training_id, guard_id, attendance) VALUES (:tid, :gid, 'Present')", {{":tid", m_trainingId}, {":gid", guardId}});
    loadParticipants();
}

void TrainingDialog::removeParticipant()
{
    auto items = m_partTable->selectedItems();
    if (items.isEmpty()) { QMessageBox::information(this, "No Selection", "Select a participant to remove."); return; }
    int id = m_partTable->item(items.first()->row(), 0)->text().toInt();
    auto& db = DatabaseManager::instance();
    db.executeNonQuery("DELETE FROM TrainingParticipant WHERE id = :id", {{":id", id}});
    loadParticipants();
}
