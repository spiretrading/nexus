#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/TimeAndSales/NoneTimeAndSalesModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/SecurityView.hpp"
#include "Spire/Ui/TransitionView.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto TITLE_NAME = QObject::tr("Time and Sales");
}

TimeAndSalesWindow::TimeAndSalesWindow(
    std::shared_ptr<ComboBox::QueryModel> securities,
    std::shared_ptr<TimeAndSalesPropertiesWindowFactory> factory,
    ModelBuilder model_builder, QWidget* parent)
    : Window(parent) {
  set_svg_icon(":/Icons/time-sales.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/time-sales.png"));
  setWindowTitle(TITLE_NAME);
  m_transition_view = new TransitionView(nullptr);
  auto security_view =
    new SecurityView(std::move(securities), *m_transition_view);
  security_view->get_current()->connect_update_signal(
    std::bind_front(&TimeAndSalesWindow::on_current, this));
  auto box = new Box(security_view);
  box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*box, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
  });
  set_body(box);
  resize(security_view->sizeHint().width(), scale_height(361));
}

void TimeAndSalesWindow::on_current(const Security& security) {
  setWindowTitle(to_text(security) + " " + QString(0x2013) + " " + TITLE_NAME);
  m_transition_view->set_status(TransitionView::Status::LOADING);
}
