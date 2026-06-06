#include "mainwindow.h"

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QStyle>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QCloseEvent>
#include <QFileInfo>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    editor(new QPlainTextEdit(this)),
    actionCut(nullptr),
    actionCopy(nullptr),
    actionPaste(nullptr)
{
    resize(760, 540);

    setCentralWidget(editor);
    editor->setPlaceholderText("Start typing your text here...");
    editor->setFocus();

    createInterface();
    createActions();
    setCurrentFile(QString());

    connect(editor->document(), &QTextDocument::contentsChanged,
            this, &MainWindow::markDocumentChanged);

    connect(editor, &QPlainTextEdit::copyAvailable,
            this, &MainWindow::updateEditActions);

    connect(QApplication::clipboard(), &QClipboard::dataChanged,
            this, &MainWindow::updateEditActions);

    updateEditActions();
    statusBar()->showMessage("Ready", 2000);
}

void MainWindow::createInterface()
{
    statusBar();
}

void MainWindow::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu("&File");
    QToolBar *fileToolBar = addToolBar("File");

    QAction *actionNew = new QAction(style()->standardIcon(QStyle::SP_FileIcon), "&New", this);
    actionNew->setShortcut(QKeySequence::New);
    connect(actionNew, &QAction::triggered, this, &MainWindow::newDocument);
    fileMenu->addAction(actionNew);
    fileToolBar->addAction(actionNew);

    QAction *actionOpen = new QAction(style()->standardIcon(QStyle::SP_DialogOpenButton), "&Open...", this);
    actionOpen->setShortcut(QKeySequence::Open);
    connect(actionOpen, &QAction::triggered, this, &MainWindow::openDocument);
    fileMenu->addAction(actionOpen);
    fileToolBar->addAction(actionOpen);

    QAction *actionSave = new QAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "&Save", this);
    actionSave->setShortcut(QKeySequence::Save);
    connect(actionSave, &QAction::triggered, this, &MainWindow::saveDocument);
    fileMenu->addAction(actionSave);
    fileToolBar->addAction(actionSave);

    QAction *actionSaveAs = new QAction("Save &As...", this);
    actionSaveAs->setShortcut(QKeySequence::SaveAs);
    connect(actionSaveAs, &QAction::triggered, this, &MainWindow::saveDocumentAs);
    fileMenu->addAction(actionSaveAs);

    fileMenu->addSeparator();

    QAction *actionExit = new QAction("E&xit", this);
    actionExit->setShortcut(QKeySequence("Ctrl+Q"));
    connect(actionExit, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(actionExit);

    QMenu *editMenu = menuBar()->addMenu("&Edit");
    QToolBar *editToolBar = addToolBar("Edit");

    actionCut = new QAction(style()->standardIcon(QStyle::SP_LineEditClearButton), "Cu&t", this);
    actionCut->setShortcut(QKeySequence::Cut);
    connect(actionCut, &QAction::triggered, editor, &QPlainTextEdit::cut);
    editMenu->addAction(actionCut);
    editToolBar->addAction(actionCut);

    actionCopy = new QAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), "&Copy", this);
    actionCopy->setShortcut(QKeySequence::Copy);
    connect(actionCopy, &QAction::triggered, editor, &QPlainTextEdit::copy);
    editMenu->addAction(actionCopy);
    editToolBar->addAction(actionCopy);

    actionPaste = new QAction(style()->standardIcon(QStyle::SP_DialogApplyButton), "&Paste", this);
    actionPaste->setShortcut(QKeySequence::Paste);
    connect(actionPaste, &QAction::triggered, editor, &QPlainTextEdit::paste);
    editMenu->addAction(actionPaste);
    editToolBar->addAction(actionPaste);

    editMenu->addSeparator();

    QAction *actionSelectAll = new QAction("Select &All", this);
    actionSelectAll->setShortcut(QKeySequence::SelectAll);
    connect(actionSelectAll, &QAction::triggered, editor, &QPlainTextEdit::selectAll);
    editMenu->addAction(actionSelectAll);

    QMenu *helpMenu = menuBar()->addMenu("&Help");

    QAction *actionAbout = new QAction("&About", this);
    connect(actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);
    helpMenu->addAction(actionAbout);
}

void MainWindow::newDocument()
{
    if (!maybeSave()) {
        statusBar()->showMessage("New document canceled", 2000);
        return;
    }

    editor->clear();
    setCurrentFile(QString());
    editor->setFocus();
    statusBar()->showMessage("New document created", 2000);
}

void MainWindow::openDocument()
{
    if (!maybeSave()) {
        statusBar()->showMessage("Open canceled", 2000);
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open File",
        QString(),
        "Text Files (*.txt);;All Files (*)"
        );

    if (fileName.isEmpty()) {
        statusBar()->showMessage("Open canceled", 2000);
        editor->setFocus();
        return;
    }

    loadFile(fileName);
    editor->setFocus();
}

bool MainWindow::saveDocument()
{
    if (currentFile.isEmpty()) {
        return saveDocumentAs();
    }

    return saveFile(currentFile);
}

bool MainWindow::saveDocumentAs()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save As",
        currentFile.isEmpty() ? "untitled.txt" : currentFile,
        "Text Files (*.txt);;All Files (*)"
        );

    if (fileName.isEmpty()) {
        statusBar()->showMessage("Save canceled", 2000);
        editor->setFocus();
        return false;
    }

    return saveFile(fileName);
}

bool MainWindow::maybeSave()
{
    if (!editor->document()->isModified()) {
        return true;
    }

    QMessageBox::StandardButton answer = QMessageBox::warning(
        this,
        "QuickPad",
        "The document has been modified.\nDo you want to save your changes?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

    if (answer == QMessageBox::Save) {
        return saveDocument();
    }

    if (answer == QMessageBox::Cancel) {
        return false;
    }

    return true;
}

bool MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "Open error", "Cannot open file.");
        return false;
    }

    QTextStream in(&file);
    editor->setPlainText(in.readAll());

    setCurrentFile(fileName);
    statusBar()->showMessage("File loaded", 2000);

    return true;
}

bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Save error", "Cannot save file.");
        return false;
    }

    QTextStream out(&file);
    out << editor->toPlainText();

    setCurrentFile(fileName);
    statusBar()->showMessage("File saved", 2000);
    editor->setFocus();

    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
    editor->document()->setModified(false);
    setWindowModified(false);
    updateWindowTitle();
}

void MainWindow::updateWindowTitle()
{
    QString fileName = currentFile.isEmpty()
    ? "untitled.txt"
    : QFileInfo(currentFile).fileName();

    setWindowTitle(fileName + "[*] - QuickPad");
}

void MainWindow::markDocumentChanged()
{
    setWindowModified(editor->document()->isModified());
}

void MainWindow::updateEditActions()
{
    bool hasSelection = editor->textCursor().hasSelection();
    bool hasClipboardText = QApplication::clipboard()->mimeData()->hasText();

    actionCut->setEnabled(hasSelection);
    actionCopy->setEnabled(hasSelection);
    actionPaste->setEnabled(hasClipboardText);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::showAboutDialog()
{
    QMessageBox::about(this,
                       "About QuickPad",
                       "QuickPad v1.0\n\nSimple text editor made with Qt Widgets.");

    editor->setFocus();
}