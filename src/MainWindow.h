#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QColorDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QFont>
#include <QModelIndexList>
#include <QSplitter>
#include <QTextEdit>
#include <QLabel>
#include <QFileDialog>
#include <QPushButton>
#include <QPixmap>
#include <QImage>
#include "OrganizerModel.h"
#include "HttpSyntaxHighlighter.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onAddFolder();
    void onAddRequest();
    void onDeleteItem();
    void onSetColor();
    void onRemoveColor();
    void onEditRequest();
    void onEditResponse();
    void onItemDoubleClicked(const QModelIndex& index);
    void onSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
    void showContextMenu(const QPoint& pos);
    void onRequestChanged();
    void onResponseChanged();
    void onImportBurp();
    void onScreenshotClicked();
    void onAddScreenshot();
    void onRemoveScreenshot();

private:
    void setupUI();
    void setupMenuBar();
    void updateRequestViewer(const QModelIndex& index);
    QModelIndex getSelectedIndex();

    QTreeView* m_treeView;
    OrganizerModel* m_model;
    QSplitter* m_splitter;
    QSplitter* m_requestResponseSplitter;
    QTextEdit* m_requestEdit;
    QTextEdit* m_responseEdit;
    QLabel* m_requestLabel;
    QLabel* m_responseLabel;
    QPushButton* m_screenshotButton;
    QPushButton* m_removeScreenshotButton;
    QModelIndex m_currentIndex;
    bool m_updatingViewer;
};

#endif // MAINWINDOW_H
