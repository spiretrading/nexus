#include "Spire/Blotter/BlotterWindow.hpp"
#include <QIcon>
#include "Spire/Blotter/BlotterStatusBar.hpp"
#include "Spire/Blotter/BlotterTabView.hpp"
#include "Spire/Blotter/BlotterTasksView.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/SplitView.hpp"

using namespace Spire;
using namespace Spire::Styles;

BlotterWindow::BlotterWindow(
    std::shared_ptr<BlotterModel> model, QWidget* parent)
    : Window(parent),
      m_model(std::move(model)) {
  set_svg_icon(":/Icons/blotter.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/blotter.png"));
  auto body = new QWidget();
  auto layout = make_vbox_layout(body);
  auto split_view = new SplitView(
    *new BlotterTasksView(m_model), *new BlotterTabView(m_model->get_tasks()));
  update_style(*split_view, [] (auto& style) {
    style.get(Any()).set(Qt::Orientation::Vertical);
  });
  layout->addWidget(split_view, 1);
  layout->addWidget(new BlotterStatusBar());
  set_body(body);
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
