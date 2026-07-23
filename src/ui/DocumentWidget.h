#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class DocumentWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DocumentWidget(QWidget* parent = nullptr);
    void refresh();
private slots:
    void addDocument();
    void editDocument();
    void deleteDocument();
    void openDocument();
    void loadDocuments();
    void filterDocuments(const QString& text);
private:
    void buildUI();
    QTableWidget* m_table;
    QLineEdit* m_searchEdit;
    QComboBox* m_typeFilter;
    QComboBox* m_entityFilter;
    QLabel* m_countLabel;
};
