#include "Spire/Ui/Dialog.hpp"
#include <QApplication>

using namespace Spire;

Dialog::Dialog(QWidget* parent)
    : Window(parent),
      m_result(QDialog::DialogCode::Rejected) {}

void Dialog::accept() {
  m_result = QDialog::DialogCode::Accepted;
  close();
}

QDialog::DialogCode Dialog::exec() {
  auto window = static_cast<QWidget*>(parent())->window();
  auto parent_center = QPoint(window->pos().x() + (window->width() / 2),
    window->pos().y() + (window->height() / 2));
  move({parent_center.x() - (width() / 2),
    parent_center.y() - (height() / 2)});
  show();
  while(isVisible()) {
    QApplication::processEvents(QEventLoop::WaitForMoreEvents);
  }
  return m_result;
}

void Dialog::reject() {
  m_result = QDialog::DialogCode::Rejected;
  close();
}
