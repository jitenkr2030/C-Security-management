#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class PhotoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PhotoWidget(QWidget* parent = nullptr);
    void refresh();
private slots:
    void addPhoto();
    void editPhoto();
    void deletePhoto();
    void viewPhoto();
    void loadPhotos();
    void filterPhotos(const QString& text);
private:
    void buildUI();
    QTableWidget* m_table;
    QLineEdit* m_searchEdit;
    QComboBox* m_categoryFilter;
    QLabel* m_countLabel;
    QLabel* m_summaryLabel;
    QLabel* m_previewLabel;
};
