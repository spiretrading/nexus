#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include <fstream>
#include <QFileDialog>
#include <QResizeEvent>
#include <QStandardPaths>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ExportTable.hpp"
#include "Spire/TimeAndSales/NoneTimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/ResponsiveLabel.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/SecurityView.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TitleBar.hpp"
#include "Spire/Ui/TransitionView.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto TITLE_NAME = QObject::tr("Time and Sales");
  const auto TITLE_SHORT_NAME = QObject::tr("T&S");

  auto update_row_style(StyleSheet& style, const Selector& selector,
      const TimeAndSalesProperties::Highlight& highlight) {
    style.get(Any() > selector).
      set(BackgroundColor(highlight.m_background_color));
    style.get(Any() > selector > is_a<TextBox>()).
      set(TextColor(highlight.m_text_color));
    return style;
  }

  struct ExportTimeAndSalesTableMoel : TableModel {
    std::shared_ptr<TimeAndSalesTableModel> m_source;

    explicit ExportTimeAndSalesTableMoel(
      std::shared_ptr<TimeAndSalesTableModel> source)
      : m_source(std::move(source)) {}

    int get_row_size() const override {
      return m_source->get_row_size() + 1;
    }

    int get_column_size() const override {
      return m_source->get_column_size();
    }

    AnyRef at(int row, int column) const override {
      auto column_id = static_cast<TimeAndSalesTableModel::Column>(column);
      if(row == 0) {
        if(column_id == TimeAndSalesTableModel::Column::TIME) {
          static const auto value = QObject::tr("Time");
          return value;
        } else if(column_id == TimeAndSalesTableModel::Column::PRICE) {
          static const auto value = QObject::tr("Price");
          return value;
        } else if(column_id == TimeAndSalesTableModel::Column::SIZE) {
          static const auto value = QObject::tr("Size");
          return value;
        } else if(column_id == TimeAndSalesTableModel::Column::MARKET) {
          static const auto value = QObject::tr("Market");
          return value;
        } else if(column_id == TimeAndSalesTableModel::Column::CONDITION) {
          static const auto value = QObject::tr("Condition");
          return value;
        } else {
          static const auto value = "";
          return value;
        }
      }
      if(column_id == TimeAndSalesTableModel::Column::MARKET) {
        static auto market =
          MarketToken(m_source->get<std::string>(row - 1, column));
        return market;
      }
      return m_source->at(row - 1, column);
    }

    connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override {
      return m_source->connect_operation_signal(slot);
    }
  };
}

TimeAndSalesWindow::TimeAndSalesWindow(
    std::shared_ptr<ComboBox::QueryModel> query_model,
    TimeAndSalesProperties properties, ModelBuilder model_builder,
    QWidget* parent)
    : Window(parent),
      m_properties(std::move(properties)),
      m_model_builder(std::move(model_builder)) {
  set_svg_icon(":/Icons/time-sales.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/time-sales.png"));
  auto labels = std::make_shared<ArrayListModel<QString>>();
  labels->push(TITLE_NAME);
  labels->push(TITLE_SHORT_NAME);
  m_responsive_title_label = new ResponsiveLabel(labels, this);
  setWindowTitle(m_responsive_title_label->get_current()->get());
  m_title_bar = static_cast<TitleBar*>(layout()->itemAt(0)->widget());
  auto title_label = m_title_bar->layout()->itemAt(1)->widget();
  title_label->installEventFilter(this);
  m_responsive_title_label->stackUnder(m_title_bar);
  m_table_view = new TimeAndSalesTableView(
    std::make_shared<TimeAndSalesTableModel>(
      std::make_shared<NoneTimeAndSalesModel>(Security())));
  m_table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_transition_view = new TransitionView(m_table_view);
  m_security_view = new SecurityView(std::move(query_model),
    *m_transition_view);
  m_security_view->get_current()->connect_update_signal(
    std::bind_front(&TimeAndSalesWindow::on_current, this));
  auto box = new Box(m_security_view);
  update_style(*box, [&] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
    style.get(Any() > is_a<TableBody>() > is_a<TextBox>()).
      set(Font(m_properties.get_font()));
    style.get(Any() > TableHeaderItem::Label()).
      set(Font(m_properties.get_font()));
    update_row_style(style, UnknownRow(),
      m_properties.get_highlight(BboIndicator::UNKNOWN));
    update_row_style(style, AboveAskRow(),
      m_properties.get_highlight(BboIndicator::ABOVE_ASK));
    update_row_style(style, AtAskRow(),
      m_properties.get_highlight(BboIndicator::AT_ASK));
    update_row_style(style, InsideRow(),
      m_properties.get_highlight(BboIndicator::INSIDE));
    update_row_style(style, AtBidRow(),
      m_properties.get_highlight(BboIndicator::AT_BID));
    update_row_style(style, BelowBidRow(),
      m_properties.get_highlight(BboIndicator::BELOW_BID));
  });
  layout()->addWidget(box);
  m_table_view->get_table()->connect_operation_signal(
    std::bind_front(&TimeAndSalesWindow::on_table_operation, this));
  m_table_view->get_table()->connect_begin_loading_signal([=] {
    if(m_transition_view->get_status() == TransitionView::Status::NONE) {
      m_transition_view->set_status(TransitionView::Status::LOADING);
    }
  });
  m_table_view->get_table()->connect_end_loading_signal([=] {
    m_transition_view->set_status(TransitionView::Status::READY);
    m_table_view->setFocus();
  });
  make_context_menu();
  resize(scale(180, 410));
}

const std::shared_ptr<TimeAndSalesModel>&
    TimeAndSalesWindow::get_model() const {
  return m_table_view->get_table()->get_model();
}

const std::shared_ptr<ComboBox::QueryModel>&
    TimeAndSalesWindow::get_query_model() const {
  return m_security_view->get_query_model();
}

const std::shared_ptr<ValueModel<Nexus::Security>>&
    TimeAndSalesWindow::get_security() const {
  return m_security_view->get_current();
}

const TimeAndSalesProperties& TimeAndSalesWindow::get_properties() const {
  return m_properties;
}

bool TimeAndSalesWindow::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Resize) {
    auto& resize_event = *static_cast<QResizeEvent*>(event);
    m_responsive_title_label->resize(resize_event.size());
    setWindowTitle(m_responsive_title_label->get_current()->get());
  }
  return Window::eventFilter(watched, event);
}

void TimeAndSalesWindow::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::RightButton &&
      m_transition_view->get_status() == TransitionView::Status::READY) {
    auto table_view = m_table_view->layout()->itemAt(0)->widget();
    auto table_header = table_view->layout()->itemAt(0)->widget();
    auto& scroll_box =
      *static_cast<ScrollBox*>(table_view->layout()->itemAt(1)->widget());
    auto scroll_bar_width = [&] {
      if(scroll_box.get_vertical_scroll_bar().isVisible()) {
        return scroll_box.get_vertical_scroll_bar().width();
      }
      return 0;
    }();
    auto scroll_bar_height = [&] {
      if(scroll_box.get_horizontal_scroll_bar().isVisible()) {
        return scroll_box.get_horizontal_scroll_bar().height();
      }
      return 0;
    }();
    auto body_rect = rect().adjusted(0,
      m_title_bar->height() + table_header->height(), -scroll_bar_width,
      -scroll_bar_height);
    if(body_rect.contains(event->pos())) {
      m_context_menu->window()->move(event->globalPos());
      m_context_menu->show();
    }
  }
}

int TimeAndSalesWindow::get_row_height() const {
  auto label = std::unique_ptr<TextBox>(make_label(""));
  update_style(*label, [&] (auto& style) {
    style.get(ReadOnly() && Disabled()).
      set(Font(m_properties.get_font())).
      set(vertical_padding(scale_height(1.5)));
  });
  return label->sizeHint().height();
}

void TimeAndSalesWindow::make_context_menu() {
  m_context_menu = new ContextMenu(*this);
  m_context_menu->add_action(tr("Properties"), [] {});
  auto link_menu = new ContextMenu(*static_cast<QWidget*>(m_context_menu));
  m_context_menu->add_menu(tr("Link to"), *link_menu);
  m_context_menu->add_separator();
  m_context_menu->add_action(tr("Export"),
    std::bind_front(&TimeAndSalesWindow::on_export, this));
  update_export_menu_item();
}

void TimeAndSalesWindow::update_export_menu_item() {
  auto& list_view =
    *static_cast<ListView*>(m_context_menu->layout()->itemAt(0)->widget());
  auto export_item =
    list_view.get_list_item(list_view.get_list()->get_size() - 1);
  if(m_table_view->get_table()->get_row_size() == 0) {
    if(export_item->isEnabled()) {
      export_item->setEnabled(false);
      update_style(*export_item, [] (auto& style) {
        style.get(Any() > is_a<TextBox>()).set(TextColor(QColor(0xC8C8C8)));
      });
    }
  } else if(!export_item->isEnabled()) {
    export_item->setEnabled(true);
    update_style(*export_item, [] (auto& style) {
      style.get(Any() > is_a<TextBox>()).set(TextColor(Qt::black));
    });
  }
}

void TimeAndSalesWindow::on_current(const Security& security) {
  auto prefix_name = to_text(security) + " " + QString(0x2013) + " ";
  m_responsive_title_label->get_labels()->set(0, prefix_name + TITLE_NAME);
  m_responsive_title_label->get_labels()->set(
    1, prefix_name + TITLE_SHORT_NAME);
  setWindowTitle(m_responsive_title_label->get_current()->get());
  m_transition_view->set_status(TransitionView::Status::NONE);
  m_table_view->get_table()->set_model(m_model_builder(security));
  auto table_view = m_table_view->layout()->itemAt(0)->widget();
  auto table_header = table_view->layout()->itemAt(0)->widget();
  m_table_view->get_table()->load_history(
    (height() - m_title_bar->height() - table_header->sizeHint().height()) /
      get_row_height());
}

void TimeAndSalesWindow::on_table_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      auto row = m_table_view->get_item({operation.m_index, 0})->parentWidget();
      row->setDisabled(true);
      row->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      switch(m_table_view->get_table()->get_bbo_indicator(operation.m_index)) {
        case BboIndicator::UNKNOWN:
          match(*row, UnknownRow());
          break;
        case BboIndicator::ABOVE_ASK:
          match(*row, AboveAskRow());
          break;
        case BboIndicator::AT_ASK:
          match(*row, AtAskRow());
          break;
        case BboIndicator::INSIDE:
          match(*row, InsideRow());
          break;
        case BboIndicator::AT_BID:
          match(*row, AtBidRow());
          break;
        case BboIndicator::BELOW_BID:
          match(*row, BelowBidRow());
          break;
      }
      update_export_menu_item();
    },
    [&] (const TableModel::RemoveOperation& operation) {
      update_export_menu_item();
    });
}

void TimeAndSalesWindow::on_export() {
  auto file_name = QFileDialog::getSaveFileName(this, tr("Export As"),
    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
      tr("/time_and_sales"), tr("CSV (*.csv)"));
  if(!file_name.isEmpty()) {
    auto out = std::ofstream(file_name.toStdString());
    export_table_as_csv(
      ExportTimeAndSalesTableMoel(m_table_view->get_table()), out);
  }
}
