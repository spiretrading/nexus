#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/TimeAndSales/NoneTimeAndSalesModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/ResponsiveLabel.hpp"
#include "Spire/Ui/SecurityView.hpp"
#include "Spire/Ui/TitleBar.hpp"
#include "Spire/Ui/TransitionView.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto TITLE_NAME = QObject::tr("Time and Sales");
  const auto TITLE_SHORT_NAME = QObject::tr("T&S");
}

TimeAndSalesWindow::TimeAndSalesWindow(
    std::shared_ptr<ComboBox::QueryModel> securities,
    std::shared_ptr<TimeAndSalesPropertiesWindowFactory> factory,
    ModelBuilder model_builder, QWidget* parent)
    : Window(parent) {
  set_svg_icon(":/Icons/time-sales.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/time-sales.png"));
  auto labels = std::make_shared<ArrayListModel<QString>>();
  labels->push(TITLE_NAME);
  labels->push(TITLE_SHORT_NAME);
  m_responsive_title_label = new ResponsiveLabel(std::move(labels), this);
  setWindowTitle(m_responsive_title_label->get_current()->get());
  m_responsive_title_label->stackUnder(layout()->itemAt(0)->widget());
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
  resize(get_title_bar().sizeHint().width(), scale_height(361));
}

void TimeAndSalesWindow::resizeEvent(QResizeEvent* event) {
  m_responsive_title_label->resize(get_title_bar().get_title_label().size());
  setWindowTitle(m_responsive_title_label->get_current()->get());
  Window::resizeEvent(event);
}

void TimeAndSalesWindow::on_current(const Security& security) {
  auto prefix_name = to_text(security) + " " + QString(0x2013) + " ";
  m_responsive_title_label->get_labels()->set(0, prefix_name + TITLE_NAME);
  m_responsive_title_label->get_labels()->set(
    1, prefix_name + TITLE_SHORT_NAME);
  setWindowTitle(m_responsive_title_label->get_current()->get());
  m_transition_view->set_status(TransitionView::Status::LOADING);
}
