#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/TimeAndSales/NoneTimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/SecurityView.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TransitionView.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using UnknownIndicator =
    StateSelector<void, struct UnknownIndicatorSeletorTag>;
  using AboveAskIndicator =
    StateSelector<void, struct AboveAskIndicatorSeletorTag>;
  using AtAskIndicator = StateSelector<void, struct AtAskIndicatorSeletorTag>;
  using InsideIndicator = StateSelector<void, struct InsideIndicatorSeletorTag>;
  using AtBidIndicator = StateSelector<void, struct AtBidIndicatorSeletorTag>;
  using BelowBidIndicator =
    StateSelector<void, struct BelowBidIndicatorSeletorTag>;
  const auto TITLE_NAME = QObject::tr("Time and Sales");

  auto get_height(const QFont& font) {
    auto label = std::unique_ptr<TextBox>(make_label("x"));
    update_style(*label, [&] (auto& style) {
      style.get(Any()).
        set(Font(font)).
        set(vertical_padding(scale_height(1.5)));
    });
    return label->sizeHint().height();
  }

  auto get_bbo_indicator_selector(BboIndicator indicator) {
    static auto selectors = std::array<Selector, BBO_INDICATOR_COUNT>{
      Any() > is_a<TableBody>() > UnknownIndicator(),
      Any() > is_a<TableBody>() > AboveAskIndicator(),
      Any() > is_a<TableBody>() > AtAskIndicator(),
      Any() > is_a<TableBody>() > InsideIndicator(),
      Any() > is_a<TableBody>() > AtBidIndicator(),
      Any() > is_a<TableBody>() > BelowBidIndicator()};
    return selectors[static_cast<int>(indicator)];
  }

  void apply_indicator_style(StyleSheet& style, const Selector& selector,
      const HighlightColor& highlight) {
    style.get(selector).
      set(BackgroundColor(highlight.m_background_color));
    style.get(selector >> is_a<TextBox>()).
      set(TextColor(highlight.m_text_color));
  };
}

TimeAndSalesWindow::TimeAndSalesWindow(
    std::shared_ptr<ComboBox::QueryModel> securities,
    std::shared_ptr<TimeAndSalesPropertiesWindowFactory> factory,
    ModelBuilder model_builder, QWidget* parent)
    : Window(parent),
      m_factory(std::move(factory)),
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
  m_body = new Box(security_view);
  m_body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*m_body, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
  });
  set_body(m_body);
  resize(security_view->sizeHint().width(), scale_height(361));
  m_table_model->connect_begin_loading_signal([=] {
    if(m_transition_view->get_status() == TransitionView::Status::NONE) {
      m_transition_view->set_status(TransitionView::Status::LOADING);
    }
  });
  m_table_model->connect_end_loading_signal([=] {
    m_transition_view->set_status(TransitionView::Status::READY);
    make_table_header_menu();
  });
  m_table_view->get_table()->connect_operation_signal(
    std::bind_front(&TimeAndSalesWindow::on_table_operation, this));
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

int TimeAndSalesWindow::get_row_height() const {
  if(m_table_model->get_row_size() == 0) {
    return get_height(m_factory->make()->get_current()->get().get_font());
  }
  return m_table_view->get_body().get_item({0, 0})->height();
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
  m_transition_view->set_status(TransitionView::Status::NONE);
  m_table_model->set_model(m_model_builder(security));
  auto& header = m_table_view->get_header();
  m_table_model->load_history(
    (m_body->height() - header.height()) / get_row_height());
  auto& properties = m_factory->make()->get_current()->get();
  update_style(*m_table_view, [&] (auto& style) {
    auto header_item_selector =
      Any() > is_a<TableHeader>() > is_a<TableHeaderItem>();
    auto body_item_selector =
      Any() > is_a<TableBody>() > Row() > is_a<TableItem>();
    style.get(header_item_selector > TableHeaderItem::Label()).
      set(Font(properties.get_font()));
    style.get(body_item_selector >> is_a<TextBox>()).
      set(Font(properties.get_font()));
    for(auto i = 0; i < BBO_INDICATOR_COUNT; ++i) {
      auto indicator = static_cast<BboIndicator>(i);
      apply_indicator_style(style, get_bbo_indicator_selector(indicator),
        properties.get_highlight_color(indicator));
    }
  });
}

void TimeAndSalesWindow::on_header_item_check(int column, bool checked) {
  m_table_view->get_header().get_item(column)->setVisible(checked);
}

void TimeAndSalesWindow::on_table_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      auto row = m_table_view->get_body().get_item(
        {operation.m_index, 0})->parentWidget();
      row->setDisabled(true);
      auto indicator = m_table_model->get_bbo_indicator(operation.m_index);
      if(indicator == BboIndicator::UNKNOWN) {
        match(*row, UnknownIndicator());
      } else if(indicator == BboIndicator::ABOVE_ASK) {
        match(*row, AboveAskIndicator());
      } else if(indicator == BboIndicator::AT_ASK) {
        match(*row, AtAskIndicator());
      } else if(indicator == BboIndicator::INSIDE) {
        match(*row, InsideIndicator());
      } else if(indicator == BboIndicator::AT_BID) {
        match(*row, AtBidIndicator());
      } else if(indicator == BboIndicator::BELOW_BID) {
        match(*row, BelowBidIndicator());
      }
    });
}
