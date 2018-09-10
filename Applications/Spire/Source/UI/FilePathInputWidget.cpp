#include "Spire/UI/FilePathInputWidget.hpp"
#include "ui_FilePathInputWidget.h"

using namespace Spire;
using namespace Spire::UI;
using namespace std;
using namespace std::filesystem;

FilePathInputWidget::FilePathInputWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_FilePathInputWidget>()),
      m_acceptMode(QFileDialog::AcceptOpen) {
  m_ui->setupUi(this);
  connect(m_ui->m_openFileButton, &QToolButton::clicked, this,
    &FilePathInputWidget::OnOpenFileButtonClicked);
}

FilePathInputWidget::~FilePathInputWidget() {}

path FilePathInputWidget::GetPath() const {
  path p(m_ui->m_pathInput->text().toStdString());
  return p;
}

void FilePathInputWidget::SetPath(const path& path) {
  m_ui->m_pathInput->setText(QString::fromStdString(path.generic_string()));
}

void FilePathInputWidget::SetCaption(const string& caption) {
  m_caption = caption;
}

void FilePathInputWidget::SetFilter(const string& filter) {
  m_filter = filter;
}

QFileDialog::AcceptMode FilePathInputWidget::GetAcceptMode() const {
  return m_acceptMode;
}

void FilePathInputWidget::SetAcceptMode(QFileDialog::AcceptMode acceptMode) {
  m_acceptMode = acceptMode;
}

void FilePathInputWidget::OnOpenFileButtonClicked() {
  QString path;
  if(m_acceptMode == QFileDialog::AcceptOpen) {
    path = QFileDialog::getOpenFileName(this, QString::fromStdString(m_caption),
      m_ui->m_pathInput->text(), QString::fromStdString(m_filter));
  } else {
    path = QFileDialog::getSaveFileName(this, QString::fromStdString(m_caption),
      m_ui->m_pathInput->text(), QString::fromStdString(m_filter));
  }
  if(path.isNull()) {
    return;
  }
  m_ui->m_pathInput->setText(path);
}
