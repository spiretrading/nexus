#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include <QFileDialog>
#include <QScreen>
#include <QStandardPaths>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ExportTable.hpp"
#include "Spire/TimeAndSales/NoneTimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindowSettings.hpp"
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
  using ShowGrid = StateSelector<void, struct ShowGridSeletorTag>;
  const auto TITLE_NAME = QObject::tr("Time and Sales");
  const auto CELL_VERTICAL_PADDING = 1.5;
  const auto DEBOUNCE_TIME_MS = 100;
  const auto EXPORT_MENU_ITEM_INDEX = 3;

  auto& get_bbo_indicator_selector(BboIndicator indicator) {
    static auto selectors = std::array<Selector, BBO_INDICATOR_COUNT>{
      UnknownIndicator(), AboveAskIndicator(), AtAskIndicator(),
        InsideIndicator(), AtBidIndicator(), BelowBidIndicator()};
    return selectors[static_cast<int>(indicator)];
  }

  void apply_highlight_style(const Selector& selector,
      const HighlightColor& highlight, StyleSheet& style) {
    auto item_selector = Any() > is_a<TableBody>() > Row() > is_a<TableItem>();
    style.get(item_selector > selector).
      set(TextColor(highlight.m_text_color));
    style.get(item_selector > (selector < is_a<TableItem>() < Row())).
      set(BackgroundColor(highlight.m_background_color));
  };

  void apply_font_style(const TimeAndSalesProperties& properties,
      StyleSheet& style) {
    auto header_item_selector =
      Any() > is_a<TableHeader>() > is_a<TableHeaderItem>();
    auto body_item_selector =
      Any() > is_a<TableBody>() > Row() > is_a<TableItem>();
    style.get(header_item_selector > TableHeaderItem::Label()).
      set(Font(properties.get_font()));
    style.get(body_item_selector > is_a<TextBox>()).
      set(Font(properties.get_font()));
  };

  auto apply_table_view_style(const TimeAndSalesProperties& properties,
      StyleSheet& style) {
    style.get(Any() > is_a<TableBody>()).
      set(grid_color(QColor(0xE0E0E0)));
    style.get(ShowGrid() > is_a<TableBody>()).
      set(HorizontalSpacing(scale_width(1))).
      set(VerticalSpacing(scale_height(1)));
    apply_font_style(properties, style);
    for(auto i = 0; i < BBO_INDICATOR_COUNT; ++i) {
      auto indicator = static_cast<BboIndicator>(i);
      apply_highlight_style(get_bbo_indicator_selector(indicator),
        properties.get_highlight_color(indicator), style);
    }
  }

  auto estimate_row_height(const QFont& font) {
    static auto padding = 2 * scale_height(CELL_VERTICAL_PADDING);
    return std::max(0, QFontMetrics(font).height()) + padding;
  }
}

TimeAndSalesWindow::TimeAndSalesWindow(
  std::shared_ptr<ComboBox::QueryModel> securities,
  std::shared_ptr<TimeAndSalesPropertiesWindowFactory> factory,
  ModelBuilder model_builder, QWidget* parent)
  : TimeAndSalesWindow(std::move(securities), std::move(factory),
      std::move(model_builder), std::string(), parent) {}

TimeAndSalesWindow::TimeAndSalesWindow(
    std::shared_ptr<ComboBox::QueryModel> securities,
    std::shared_ptr<TimeAndSalesPropertiesWindowFactory> factory,
    ModelBuilder model_builder, std::string identifier, QWidget* parent)
    : Window(parent),
      SecurityContext(std::move(identifier)),
      m_factory(std::move(factory)),
      m_model_builder(std::move(model_builder)),
      m_table_model(std::make_shared<TimeAndSalesTableModel>(
        std::make_shared<NoneTimeAndSalesModel>())),
      m_table_view(nullptr),
      m_timer(this) {
  set_svg_icon(":/Icons/time-sales.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/time-sales.png"));
  setWindowTitle(TITLE_NAME);
  m_transition_view = new TransitionView(new QWidget());
  m_security_view = new SecurityView(std::move(securities), *m_transition_view);
  m_security_view->get_current()->connect_update_signal(
    std::bind_front(&TimeAndSalesWindow::on_current, this));
  m_security_view->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_security_view->setContextMenuPolicy(Qt::CustomContextMenu);
  set_body(m_security_view);
  update_style(*this, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
  });
  connect(m_security_view, &QWidget::customContextMenuRequested,
    std::bind_front(&TimeAndSalesWindow::on_context_menu, this,
      m_security_view));
  m_properties_connection = m_factory->get_properties()->connect_update_signal(
    std::bind_front(&TimeAndSalesWindow::on_properties, this));
  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout,
    std::bind_front(&TimeAndSalesWindow::on_timeout, this));
  resize(m_security_view->sizeHint().width(), scale_height(361));
}

std::unique_ptr<LegacyUI::WindowSettings>
    TimeAndSalesWindow::GetWindowSettings() const {
  return std::make_unique<TimeAndSalesWindowSettings>(*this);
}

void TimeAndSalesWindow::HandleLink(SecurityContext& context) {
  m_link_identifier = context.GetIdentifier();
  m_link_connection = context.ConnectSecurityDisplaySignal(
    [=] (const auto& security) {
      if(m_security_view->get_current()->get() != security) {
         m_security_view->get_current()->set(security);
      }
    });
  m_security_view->get_current()->set(context.GetDisplayedSecurity());
}

void TimeAndSalesWindow::HandleUnlink() {
  m_link_connection.disconnect();
  m_link_identifier.clear();
}

void TimeAndSalesWindow::update_grid(const TimeAndSalesProperties& properties) {
  if(properties.is_grid_enabled()) {
    match(*m_table_view, ShowGrid());
  } else {
    unmatch(*m_table_view, ShowGrid());
  }
}

void TimeAndSalesWindow::on_context_menu(QWidget* parent, const QPoint& pos) {
  auto menu = new ContextMenu(*parent);
  menu->add_action(tr("Properties"),
    std::bind_front(&TimeAndSalesWindow::on_properties_menu, this));
  auto link_menu = new ContextMenu(*static_cast<QWidget*>(menu));
  menu->add_menu(tr("Link to"), *link_menu);
  menu->add_separator();
  menu->add_action(tr("Export..."),
    std::bind_front(&TimeAndSalesWindow::on_export_menu, this));
  if(auto export_item = menu->get_menu_item(EXPORT_MENU_ITEM_INDEX)) {
    export_item->setEnabled(m_table_model->get_row_size() != 0);
  }
  menu->window()->setAttribute(Qt::WA_DeleteOnClose);
  menu->window()->move(parent->mapToGlobal(pos));
  menu->window()->show();
}

void TimeAndSalesWindow::on_export_menu() {
  auto file_name = QFileDialog::getSaveFileName(this, tr("Save As"),
    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
    tr("/time_and_sales"), tr("CSV (*.csv)"));
  if(!file_name.isEmpty()) {
    auto out = std::ofstream(file_name.toStdString());
    auto items = m_table_view->get_header().get_items();
    auto headers = std::vector<QString>(items->get_size() - 1);
    std::transform(items->begin(), items->end() - 1, headers.begin(),
      [] (const TableHeaderItem::Model& item) {
        return item.m_name;
      });
    export_table_as_csv(*m_table_model, headers, out);
  }
}

void TimeAndSalesWindow::on_properties_menu() {
  auto properties_window = m_factory->make();
  if(!properties_window->isVisible()) {
    properties_window->show();
    if(screen()->geometry().right() - frameGeometry().right() >=
        properties_window->frameGeometry().width()) {
      properties_window->move(frameGeometry().right(), y());
    } else {
      properties_window->move(x() - properties_window->frameGeometry().width(),
        y());
    }
  }
  properties_window->activateWindow();
}

void TimeAndSalesWindow::on_begin_loading() {
  if(m_transition_view->get_status() == TransitionView::Status::NONE) {
    m_transition_view->set_status(TransitionView::Status::LOADING);
  }
}

void TimeAndSalesWindow::on_end_loading() {
  m_transition_view->set_status(TransitionView::Status::READY);
}

void TimeAndSalesWindow::on_current(const Security& security) {
  if(security == Security()) {
    return;
  }
  setWindowTitle(to_text(security) + " " + QString(0x2013) + " " + TITLE_NAME);
  auto column_widths = [&] () -> std::shared_ptr<ListModel<int>> {
    if(m_table_view) {
      m_table_view->get_header().get_widths();
    }
    return nullptr;
  }();
  m_transition_view->set_status(TransitionView::Status::NONE);
  m_table_model =
    std::make_shared<TimeAndSalesTableModel>(m_model_builder(security));
  m_table_model->connect_begin_loading_signal(
    std::bind_front(&TimeAndSalesWindow::on_begin_loading, this));
  m_table_model->connect_end_loading_signal(
    std::bind_front(&TimeAndSalesWindow::on_end_loading, this));
  m_table_view = make_time_and_sales_table_view(
    m_table_model, m_factory->get_properties());
  m_table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*m_table_view, std::bind_front(apply_table_view_style,
    m_factory->make()->get_current()->get()));
  if(column_widths) {
    auto& header = m_table_view->get_header();
    for(auto i = 0; i != column_widths->get_size(); ++i) {
      header.get_widths()->set(i, column_widths->get(i));
    }
  }
  m_transition_view->set_body(*m_table_view);
  auto& properties = m_factory->make()->get_current()->get();
  update_grid(properties);
  m_table_model->load_history(
    m_security_view->height() / estimate_row_height(properties.get_font()));
  SetDisplayedSecurity(security);
}

void TimeAndSalesWindow::on_properties(
    const TimeAndSalesProperties& properties) {
  m_timer.start(DEBOUNCE_TIME_MS);
}

void TimeAndSalesWindow::on_timeout() {
  auto& properties = m_factory->make()->get_current()->get();
  if(m_table_view) {
    if(properties.get_font() != m_properties.get_font()) {
      update_style(*m_table_view,
        std::bind_front(apply_font_style, properties));
    }
    if(properties.is_grid_enabled() != m_properties.is_grid_enabled()) {
      update_grid(properties);
    }
    auto& header = m_table_view->get_header();
    for(auto i = 0; i != TimeAndSalesTableModel::COLUMN_SIZE; ++i) {
      if(properties.is_visible(
          static_cast<TimeAndSalesTableModel::Column>(i))) {
        m_table_view->show_column(i);
      } else {
        m_table_view->hide_column(i);
      }
    }
    for(auto i = 0; i < BBO_INDICATOR_COUNT; ++i) {
      auto indicator = static_cast<BboIndicator>(i);
      if(properties.get_highlight_color(indicator) !=
        m_properties.get_highlight_color(indicator)) {
        update_style(*m_table_view, std::bind_front(apply_highlight_style,
          get_bbo_indicator_selector(indicator),
          properties.get_highlight_color(indicator)));
      }
    }
  }
  m_properties = properties;
}
