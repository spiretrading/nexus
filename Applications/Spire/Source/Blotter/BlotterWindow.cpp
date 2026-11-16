#include "Spire/Blotter/BlotterWindow.hpp"
#include <QIcon>

using namespace Spire;

BlotterWindow::BlotterWindow(
    std::shared_ptr<BlotterModel> model, QWidget* parent)
    : Window(parent),
      m_model(std::move(model)) {
  set_svg_icon(":/Icons/blotter.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/blotter.png"));
  set_body(new QWidget());
  m_name_connection = m_model->get_name()->connect_update_signal(
    std::bind_front(&BlotterWindow::on_name, this));
  on_name(m_model->get_name()->get());
}

const std::shared_ptr<BlotterModel>& BlotterWindow::get_model() const {
  return m_model;
}

void BlotterWindow::on_name(const QString& name) {
  setWindowTitle(tr("Blotter \xE2\x80\x93 %1").arg(name));
}
