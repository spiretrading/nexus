#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/TimeAndSales/NoneTimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ContextMenu.hpp"
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

  auto make_header_widths() {
    auto widths = std::vector<int>();
    widths.push_back(scale_width(45));
    widths.push_back(scale_width(50));
    widths.push_back(scale_width(40));
    widths.push_back(scale_width(38));
    widths.push_back(scale_width(34));
    return widths;
  }
}

TimeAndSalesWindow::TimeAndSalesWindow(
    std::shared_ptr<ComboBox::QueryModel> securities,
    std::shared_ptr<TimeAndSalesPropertiesWindowFactory> factory,
    ModelBuilder model_builder, QWidget* parent)
    : Window(parent),
      m_model_builder(std::move(model_builder)),
      m_table_model(std::make_shared<TimeAndSalesTableModel>(
        std::make_shared<NoneTimeAndSalesModel>())),
     m_table_header_menu(nullptr) {
  set_svg_icon(":/Icons/time-sales.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/time-sales.png"));
  setWindowTitle(TITLE_NAME);
  m_table_view = make_time_and_sales_table_view(m_table_model);
  m_table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_table_view->get_header().installEventFilter(this);
  m_transition_view = new TransitionView(m_table_view);
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
  m_header_item_widths = make_header_widths();
  for(auto i = 0; i < std::ssize(m_header_item_widths); ++i) {
    //if(m_table_view->get_header().get_item(i)->isVisible()) {
      m_table_view->get_header().get_widths()->set(i, m_header_item_widths[i]);
    //}
  }
  resize(sizeHint().width(), scale_height(361));
  m_table_model->connect_begin_loading_signal([=] {
    if(m_transition_view->get_status() == TransitionView::Status::NONE) {
      m_transition_view->set_status(TransitionView::Status::LOADING);
    }
    });
  m_table_model->connect_end_loading_signal([=] {
    m_transition_view->set_status(TransitionView::Status::READY);
    make_table_header_menu();
    m_table_view->setFocus();
  });
}

bool TimeAndSalesWindow::eventFilter(QObject* watched, QEvent* event) {
  if(watched == &m_table_view->get_header()) {
    if(event->type() == QEvent::MouseButtonPress) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      if(mouse_event.button() == Qt::RightButton) {
        m_table_header_menu->window()->move(mouse_event.globalPos());
        m_table_header_menu->window()->show();
      }
    }
  }
  return Window::eventFilter(watched, event);
}

void TimeAndSalesWindow::make_table_header_menu() {
  if(m_table_header_menu) {
    return;
  }
  m_table_header_menu = new ContextMenu(*this);
  auto add_sub_menu = [&] (int column, const QString& name, bool checked) {
    auto model = m_table_header_menu->add_check_box(name);
    model->set(checked);
    model->connect_update_signal(std::bind_front(
      &TimeAndSalesWindow::on_header_item_check, this, column));
    };
  auto header_items = m_table_view->get_header().get_items();
  for(auto i = 0; i < header_items->get_size() - 1; ++i) {
    add_sub_menu(i, header_items->get(i).m_name,
      m_table_view->get_header().get_item(i)->isVisible());
  }
}

void TimeAndSalesWindow::on_current(const Security& security) {
  setWindowTitle(to_text(security) + " " + QString(0x2013) + " " + TITLE_NAME);
  //m_transition_view->set_status(TransitionView::Status::LOADING);
  m_transition_view->set_status(TransitionView::Status::NONE);
  m_table_model->set_model(m_model_builder(security));
  m_table_model->load_history(
    (height() - m_table_view->get_header().sizeHint().height()) / 24);
}

void TimeAndSalesWindow::on_header_item_check(int column, bool checked) {
  auto header_items = m_table_view->get_header().get_items();
  auto width = [&] {
    if(checked) {
      return m_header_item_widths[column];
    }
    return 0;
  }();
  m_table_view->get_header().get_item(column)->setVisible(checked);
  m_table_view->get_header().get_widths()->set(column, width);
}
