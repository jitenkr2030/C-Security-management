#include "TrainingWidget.h"
#include "TrainingDialog.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDate>
#include <QCoreApplication>
#include <QDir>

TrainingWidget::TrainingWidget(QWidget* parent) : QWidget(parent) { buildUI(); loadTrainings(); }

void TrainingWidget::buildUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Training Management");
    titleLabel->setObjectName("PageTitle");
    mainLayout->addWidget(titleLabel);
    auto* subtitle = new QLabel("Schedule training programs and track guard participation");
    subtitle->setObjectName("PageSubtitle");
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(8);

    auto* addBtn = new QPushButton("+ New Training");
    addBtn->setObjectName("PrimaryButton");
    addBtn->setFixedSize(140, 36);
    addBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &TrainingWidget::addTraining);
    headerRow->addWidget(addBtn);

    auto* editBtn = new QPushButton("Edit");
    editBtn->setObjectName("SecondaryButton");
    editBtn->setFixedSize(70, 36);
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, &TrainingWidget::editTraining);
    headerRow->addWidget(editBtn);

    auto* delBtn = new QPushButton("Delete");
    delBtn->setObjectName("DangerButton");
    delBtn->setFixedSize(80, 36);
    delBtn->setCursor(Qt::PointingHandCursor);
    connect(delBtn, &QPushButton::clicked, this, &TrainingWidget::deleteTraining);
    headerRow->addWidget(delBtn);

    auto* exportBtn = new QPushButton("Export CSV");
    exportBtn->setObjectName("SecondaryButton");
    exportBtn->setFixedSize(110, 36);
    exportBtn->setCursor(Qt::PointingHandCursor);
    connect(exportBtn, &QPushButton::clicked, this, &TrainingWidget::exportCSV);
    headerRow->addWidget(exportBtn);

    headerRow->addStretch();
    m_countLabel = new QLabel;
    m_countLabel->setStyleSheet("color: #8B95A5; font-size: 13px; font-weight: 600;");
    headerRow->addWidget(m_countLabel);
    mainLayout->addLayout(headerRow);

    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by name, trainer, location...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &TrainingWidget::filterTrainings);
    filterRow->addWidget(m_searchEdit, 1);

    m_typeFilter = new QComboBox;
    m_typeFilter->addItems({"All Types", "Fire Safety", "First Aid", "Physical Training",
                            "Drill", "Weapon Handling", "CCTV Operations",
                            "Crowd Management", "Legal Awareness", "Other"});
    m_typeFilter->setFixedWidth(160);
    connect(m_typeFilter, &QComboBox::currentTextChanged, this,
            [this](const QString&) { filterTrainings(m_searchEdit->text()); });
    filterRow->addWidget(m_typeFilter);
    mainLayout->addLayout(filterRow);

    m_summaryLabel = new QLabel;
    m_summaryLabel->setStyleSheet("color: #D4B44C; font-size: 13px; font-weight: 600;");
    mainLayout->addWidget(m_summaryLabel);

    m_table = new QTableWidget;
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(true);

    QStringList cols = {"ID", "Training Name", "Type", "Trainer", "Start", "End",
                        "Location", "Participants", "Notes"};
    m_table->setColumnCount(cols.size());
    m_table->setHorizontalHeaderLabels(cols);
    m_table->setColumnHidden(0, true);
    m_table->setColumnWidth(1, 180);
    m_table->setColumnWidth(2, 130);
    m_table->setColumnWidth(3, 140);
    m_table->setColumnWidth(4, 100);
    m_table->setColumnWidth(5, 100);
    m_table->setColumnWidth(6, 140);
    m_table->setColumnWidth(7, 100);
    m_table->horizontalHeader()->setStretchLastSection(true);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &TrainingWidget::editTraining);
    mainLayout->addWidget(m_table, 1);
}

void TrainingWidget::loadTrainings()
{
    auto& db = DatabaseManager::instance();
    auto query = db.execute("SELECT * FROM Training ORDER BY start_date DESC");
    int count = 0;
    auto cc = db.execute("SELECT COUNT(*) FROM Training");
    if (cc.next()) count = cc.value(0).toInt();
    m_table->setRowCount(count);
    m_table->setSortingEnabled(false);

    int row = 0;
    int totalParticipants = 0;

    while (query.next()) {
        int tid = query.value("id").toInt();
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(row, col, item);
        };
        setItem(0, QString::number(tid));
        setItem(1, query.value("training_name").toString());
        setItem(2, query.value("training_type").toString());
        setItem(3, query.value("trainer_name").toString());
        setItem(4, query.value("start_date").toString());
        setItem(5, query.value("end_date").toString());
        setItem(6, query.value("location").toString());

        auto pq = db.execute("SELECT COUNT(*) FROM TrainingParticipant WHERE training_id = :tid",
                             {{":tid", tid}});
        int partCount = 0;
        if (pq.next()) partCount = pq.value(0).toInt();
        totalParticipants += partCount;
        auto* partItem = new QTableWidgetItem(QString::number(partCount));
        partItem->setTextAlignment(Qt::AlignCenter);
        partItem->setForeground(QColor("#D4B44C"));
        m_table->setItem(row, 7, partItem);

        setItem(8, query.value("notes").toString());
        row++;
    }
    m_table->setSortingEnabled(true);
    m_countLabel->setText(QString("%1 trainings").arg(row));
    m_summaryLabel->setText(QString("Total Trainings: %1 | Total Participants: %2")
                                .arg(row).arg(totalParticipants));
}

void TrainingWidget::refresh() { loadTrainings(); }

void TrainingWidget::addTraining()
{
    TrainingDialog dlg(this, -1);
    if (dlg.exec() == QDialog::Accepted) loadTrainings();
}

void TrainingWidget::editTraining()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Select a training to edit.");
        return;
    }
    int id = m_table->item(items.first()->row(), 0)->text().toInt();
    TrainingDialog dlg(this, id);
    if (dlg.exec() == QDialog::Accepted) loadTrainings();
}

void TrainingWidget::deleteTraining()
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Select a training to delete.");
        return;
    }
    int row = items.first()->row();
    int id = m_table->item(row, 0)->text().toInt();
    QString name = m_table->item(row, 1)->text();
    auto result = QMessageBox::question(this, "Delete",
        QString("Delete training \"%1\"?\nAll participant records will also be deleted.").arg(name),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (result == QMessageBox::Yes) {
        auto& db = DatabaseManager::instance();
        db.executeNonQuery("DELETE FROM Training WHERE id = :id", {{":id", id}});
        loadTrainings();
    }
}

void TrainingWidget::filterTrainings(const QString& text)
{
    QString searchText = text.toLower();
    QString typeFilter = m_typeFilter->currentText();
    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool textMatch = searchText.isEmpty();
        bool typeMatch = (typeFilter == "All Types");
        if (!textMatch) {
            for (int col = 1; col <= 6; ++col) {
                auto* item = m_table->item(row, col);
                if (item && item->text().toLower().contains(searchText)) {
                    textMatch = true; break;
                }
            }
        }
        if (!typeMatch) {
            auto* item = m_table->item(row, 2);
            if (item) typeMatch = (item->text() == typeFilter);
        }
        m_table->setRowHidden(row, !(textMatch && typeMatch));
    }
}

void TrainingWidget::exportCSV()
{
    if (m_table->rowCount() == 0) {
        QMessageBox::information(this, "No Data", "No training data to export.");
        return;
    }
    QDir().mkpath(QCoreApplication::applicationDirPath() + "/reports");
    QString filePath = QFileDialog::getSaveFileName(
        this, "Export Training Report",
        QCoreApplication::applicationDirPath() + "/reports/trainings.csv",
        "CSV Files (*.csv);;All Files (*)");
    if (filePath.isEmpty()) return;
    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file.");
        return;
    }
    QTextStream out(&file);
    QStringList headers;
    QList<int> visibleCols;
    for (int col = 0; col < m_table->columnCount(); ++col) {
        if (!m_table->isColumnHidden(col)) {
            visibleCols << col;
            auto* h = m_table->horizontalHeaderItem(col);
            headers << (h ? h->text() : "");
        }
    }
    out << headers.join(",") << "\n";
    for (int row = 0; row < m_table->rowCount(); ++row) {
        if (m_table->isRowHidden(row)) continue;
        QStringList rowParts;
        for (int col : visibleCols) {
            auto* item = m_table->item(row, col);
            QString text = item ? item->text() : "";
            if (text.contains(',') || text.contains('"'))
                text = "\"" + text.replace("\"", "\"\"") + "\"";
            rowParts << text;
        }
        out << rowParts.join(",") << "\n";
    }
    file.close();
    QMessageBox::information(this, "Export Successful",
        QString("Training report exported to:\n\n%1").arg(filePath));
}
