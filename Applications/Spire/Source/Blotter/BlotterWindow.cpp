#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/Blotter/BlotterExecutionsView.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterOrderLogView.hpp"
#include "Spire/Blotter/BlotterPositionsView.hpp"
#include "Spire/Blotter/BlotterProfitAndLossView.hpp"
#include "Spire/Blotter/BlotterStatusBar.hpp"
#include "Spire/Blotter/BlotterTaskView.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/SplitView.hpp"
#include "Spire/Ui/TabView.hpp"

using namespace Spire;
using namespace Spire::Styles;

BlotterWindow::BlotterWindow(
    std::shared_ptr<BlotterModel> blotter, QWidget* parent)
    : Window(parent),
      m_blotter(std::move(blotter)) {
  set_svg_icon(":/Icons/blotter.svg");
  auto body = new QWidget();
  set_body(body);
  auto tabs = new TabView();
  auto positions = new BlotterPositionsView(m_blotter->get_positions());
  tabs->add(tr("Positions"), *positions);
  auto order_log = new BlotterOrderLogView(m_blotter->get_orders());
  tabs->add(tr("Order Log"), *order_log);
  auto executions = new BlotterExecutionsView(m_blotter->get_orders());
  tabs->add(tr("Executions"), *executions);
  auto profit_and_loss = new BlotterProfitAndLossView();
  tabs->add(tr("Profit/Loss"), *profit_and_loss);
  auto tasks = new BlotterTaskView();
  auto split_view = new SplitView(*tasks, *tabs);
  update_style(*split_view, [] (auto& styles) {
    styles.get(Any()).set(Qt::Orientation::Vertical);
  });
  auto layout = make_vbox_layout(body);
  layout->addWidget(split_view);
  auto status_bar = new BlotterStatusBar(m_blotter->get_status());
  layout->addWidget(status_bar);
  m_name_connection = m_blotter->get_name()->connect_update_signal(
    std::bind_front(&BlotterWindow::on_name_update, this));
  on_name_update(m_blotter->get_name()->get());
}

void BlotterWindow::on_name_update(const QString& name) {
  if(name.isEmpty()) {
    setWindowTitle(tr("Blotter"));
  } else {
    setWindowTitle(tr("Blotter") + " - " + name);
  }
}
