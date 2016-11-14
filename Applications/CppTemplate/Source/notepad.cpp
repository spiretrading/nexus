#include "CppTemplate/notepad.hpp"
#include "ui_notepad.h"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

Notepad::Notepad(QWidget *parent):QMainWindow(parent),ui(new Ui::Notepad){

  ui->setupUi(this);
  connect(ui->actionOpen,&QAction::triggered, this, &Notepad::on_actionOpen_triggered);
  connect(ui->actionSave,&QAction::triggered, this, &Notepad::on_actionSave_triggered);
}

void Notepad::on_cancelButton_clicked()
{
  qApp->quit();
}

Notepad::~Notepad()
{
  delete ui;
}

void Notepad::on_actionOpen_triggered()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
    tr("Text Files (*.txt);;C++ Files (*.cpp *.h)"));

  if (!fileName.isEmpty()) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
      QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
      return;
    }
    QTextStream in(&file);
    ui->textEdit->setText(in.readAll());
    file.close();
  }
}

void Notepad::on_actionSave_triggered()
{
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QString(),
    tr("Text Files (*.txt);;C++ Files (*.cpp *.h)"));

  if (!fileName.isEmpty()) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
      // error message
    }
    else {
      QTextStream stream(&file);
      stream << ui->textEdit->toPlainText();
      stream.flush();
      file.close();
    }
  }
}