#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QString>
#include <QAction>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newDocument();
    void openDocument();
    bool saveDocument();
    bool saveDocumentAs();
    void showAboutDialog();
    void markDocumentChanged();
    void updateEditActions();

private:
    QPlainTextEdit *editor;
    QString currentFile;

    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;

    void createInterface();
    void createActions();
    void updateWindowTitle();
    void setCurrentFile(const QString &fileName);

    bool maybeSave();
    bool loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
};

#endif