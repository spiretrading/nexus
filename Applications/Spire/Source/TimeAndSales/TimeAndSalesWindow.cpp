#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include <QFileDialog>
#include <QStandardPaths>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ExportTable.hpp"
#include "Spire/TimeAndSales/NoneTimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/SecurityView.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TransitionView.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto TITLE_NAME = QObject::tr("Time and Sales");
  const auto CELL_VERTICAL_PADDING = 1.5;

  auto& get_bbo_indicator_selector(BboIndicator indicator) {
    static auto selectors = std::array<Selector, BBO_INDICATOR_COUNT>{
      UnknownIndicator(), AboveAskIndicator(), AtAskIndicator(),
        InsideIndicator(), AtBidIndicator(), BelowBidIndicator()};
    return selectors[static_cast<int>(indicator)];
  }

  void apply_highlight_style(StyleSheet& style, const Selector& selector,
      const HighlightColor& highlight) {
    auto item_selector = Any() > is_a<TableBody>() > Row() > is_a<TableItem>();
    style.get(item_selector > selector).
      set(TextColor(highlight.m_text_color));
    style.get(item_selector > (selector < is_a<TableItem>() < Row())).
      set(BackgroundColor(highlight.m_background_color));
  };

  auto apply_table_view_style(const TimeAndSalesProperties& properties,
      StyleSheet& style) {
    auto header_item_selector =
      Any() > is_a<TableHeader>() > is_a<TableHeaderItem>();
    auto body_item_selector =
      Any() > is_a<TableBody>() > Row() > is_a<TableItem>();
    style.get(header_item_selector > TableHeaderItem::Label()).
      set(Font(properties.get_font()));
    style.get(body_item_selector > is_a<TextBox>()).
      set(Font(properties.get_font()));
    for(auto i = 0; i < BBO_INDICATOR_COUNT; ++i) {
      auto indicator = static_cast<BboIndicator>(i);
      apply_highlight_style(style, get_bbo_indicator_selector(indicator),
        properties.get_highlight_color(indicator));
    }
  }

  auto get_height(const QFont& font) {
    auto height = QFontMetrics(font).height() +
      2 * scale_height(CELL_VERTICAL_PADDING);
    if(height <= 0) {
      height = 2 * scale_height(CELL_VERTICAL_PADDING);
    }
    return height;
  }
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
  update_style(*m_table_view, std::bind_front(apply_table_view_style,
    m_factory->make()->get_current()->get()));
  auto scroll_box = new ScrollBox(m_table_view);
  scroll_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  scroll_box->set_horizontal(ScrollBox::DisplayPolicy::ON_ENGAGE);
  scroll_box->set_vertical(ScrollBox::DisplayPolicy::NEVER);
  m_transition_view = new TransitionView(scroll_box);
  auto security_view =
    new SecurityView(std::move(securities), *m_transition_view);
  security_view->get_current()->connect_update_signal(
    std::bind_front(&TimeAndSalesWindow::on_current, this));
  m_body = new Box(security_view);
  m_body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_body->installEventFilter(this);
  update_style(*m_body, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
  });
  set_body(m_body);
  resize(security_view->sizeHint().width(), scale_height(361));
  m_body_menu = new ContextMenu(*m_body);
  m_body_menu->add_action(tr("Properties"),
    std::bind_front(&TimeAndSalesWindow::on_properties_menu, this));
  auto link_menu = new ContextMenu(*static_cast<QWidget*>(m_body_menu));
  m_body_menu->add_menu(tr("Link to"), *link_menu);
  m_body_menu->add_separator();
  m_body_menu->add_action(tr("Export..."),
    std::bind_front(&TimeAndSalesWindow::on_export_menu, this));
  m_table_model->connect_begin_loading_signal(
    std::bind_front(&TimeAndSalesWindow::on_begin_loading, this));
  m_table_model->connect_end_loading_signal(
    std::bind_front(&TimeAndSalesWindow::on_end_loading, this));
}

bool TimeAndSalesWindow::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::MouseButtonPress) {
    auto& mouse_event = *static_cast<QMouseEvent*>(event);
    if(mouse_event.button() == Qt::RightButton) {
      if(watched == &m_table_view->get_header()) {
        m_table_header_menu->window()->move(mouse_event.globalPos());
        m_table_header_menu->window()->show();
        return true;
      } else if(watched == m_body) {
        m_body_menu->window()->move(mouse_event.globalPos());
        m_body_menu->window()->show();
        return true;
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
  auto header_items = m_table_view->get_header().get_items();
  for(auto i = 0; i < header_items->get_size() - 1; ++i) {
    auto model =
      m_table_header_menu->add_check_box(header_items->get(i).m_name);
    model->set(m_table_view->get_header().get_item(i)->isVisible());
    model->connect_update_signal(std::bind_front(
      &TimeAndSalesWindow::on_header_item_check, this, i));
  }
}

void TimeAndSalesWindow::on_export_menu() {
  auto file_name = QFileDialog::getSaveFileName(this, tr("Save As"),
    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
    tr("/time_and_sales"), tr("CSV (*.csv)"));
  if(!file_name.isEmpty()) {
    auto out = std::ofstream(file_name.toStdString());
    export_table_as_csv(*m_table_model,
      {tr("Time"), tr("Price"), tr("Size"), tr("Market"), tr("Condition")},
      out);
  }
}

void TimeAndSalesWindow::on_properties_menu() {
}

void TimeAndSalesWindow::on_begin_loading() {
  if(m_transition_view->get_status() == TransitionView::Status::NONE) {
    m_transition_view->set_status(TransitionView::Status::LOADING);
  }
}

void TimeAndSalesWindow::on_end_loading() {
  m_transition_view->set_status(TransitionView::Status::READY);
  make_table_header_menu();
}

void TimeAndSalesWindow::on_current(const Security& security) {
  setWindowTitle(to_text(security) + " " + QString(0x2013) + " " + TITLE_NAME);
  m_transition_view->set_status(TransitionView::Status::NONE);
  m_table_model->set_model(m_model_builder(security));
  auto& header = m_table_view->get_header();
  auto& properties = m_factory->make()->get_current()->get();
  m_table_model->load_history((m_body->height() - header.sizeHint().height()) /
    get_height(properties.get_font()));
}

void TimeAndSalesWindow::on_header_item_check(int column, bool checked) {
  m_table_view->get_header().get_item(column)->setVisible(checked);
}
