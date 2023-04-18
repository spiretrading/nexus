#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include <QResizeEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/TimeAndSales/NoneTimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ResponsiveLabel.hpp"
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
}

TimeAndSalesWindow::TimeAndSalesWindow(
    std::shared_ptr<ComboBox::QueryModel> query_model,
    TimeAndSalesWindowProperties properties, ModelBuilder model_builder,
    QWidget* parent)
    : Window(parent),
      m_properties(std::move(properties)),
      m_model_builder(std::move(model_builder)),
      m_is_updating_model(false) {
  auto labels = std::make_shared<ArrayListModel<QString>>();
  labels->push(TITLE_NAME);
  labels->push(TITLE_SHORT_NAME);
  m_title_label = new ResponsiveLabel(labels, this);
  setWindowTitle(m_title_label->get_current()->get());
  m_title_bar = static_cast<TitleBar*>(layout()->itemAt(0)->widget());
  m_title_bar->layout()->itemAt(1)->widget()->installEventFilter(this);
  m_title_label->stackUnder(m_title_bar);
  set_svg_icon(":/Icons/time-sales.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/time-sales.png"));
  resize(scale(180, 410));
  m_table_view = new TimeAndSalesTableView(
    std::make_shared<TimeAndSalesTableModel>(
      std::make_shared<NoneTimeAndSalesModel>(Security())));
  m_transition_view = new TransitionView(m_table_view);
  m_transition_view->set_status(TransitionView::Status::NONE);
  m_security_view = new SecurityView(std::move(query_model),
    std::make_shared<LocalValueModel<Security>>(), *m_transition_view);
  m_current_connection = m_security_view->get_current()->connect_update_signal(
    std::bind_front(&TimeAndSalesWindow::on_current, this));
  auto box = new Box(m_security_view);
  update_style(*box, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
  });
  layout()->addWidget(box);
  m_table_view->get_table()->connect_operation_signal(std::bind_front(&TimeAndSalesWindow::on_table_operation, this));
  m_table_view->get_table()->connect_begin_loading_signal([=] {
    if(m_is_updating_model) {
      m_transition_view->set_status(TransitionView::Status::LOADING);
    }
  });
  m_table_view->get_table()->connect_end_loading_signal([=] {
    if(m_is_updating_model) {
      m_transition_view->set_status(TransitionView::Status::READY);
      m_is_updating_model = false;
    }
  });
  make_context_menu();
}

const std::shared_ptr<TimeAndSalesModel>& TimeAndSalesWindow::get_model() const {
  return m_table_view->get_table()->get_model();
}

const std::shared_ptr<ComboBox::QueryModel>& TimeAndSalesWindow::get_query_model() const {
  return m_security_view->get_query_model();
}

const std::shared_ptr<ValueModel<Nexus::Security>>& TimeAndSalesWindow::get_security() const {
  return m_security_view->get_current();
}

const TimeAndSalesWindowProperties& TimeAndSalesWindow::get_properties() const {
  return m_properties;
}

bool TimeAndSalesWindow::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Resize) {
    auto& resize_event = *static_cast<QResizeEvent*>(event);
    m_title_label->resize(resize_event.size());
    setWindowTitle(m_title_label->get_current()->get());
  }
  return Window::eventFilter(watched, event);
}

void TimeAndSalesWindow::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::RightButton && !m_table_view->isHidden()) {
    auto table_header = static_cast<TableHeader*>(static_cast<Box*>(
      m_table_view->layout()->itemAt(0)->widget()->layout()->itemAt(0)->widget())->get_body()->layout()->
      itemAt(0)->widget());
    if((rect().adjusted(0, m_title_bar->height() + table_header->height(), 0, 0).contains(event->pos()))) {
      m_context_menu->window()->move(event->globalPos());
      m_context_menu->show();
    }
  }
}

void TimeAndSalesWindow::make_context_menu() {
  m_context_menu = new ContextMenu(*this);
  m_context_menu->add_action(tr("Properties"), [] {});
  auto link_menu = new ContextMenu(*static_cast<QWidget*>(m_context_menu));
  m_context_menu->add_menu(tr("Link to"), *link_menu);
  m_context_menu->add_separator();
  m_context_menu->add_action(tr("Export"), [] {});
  update_export_menu_item();
}

void TimeAndSalesWindow::update_export_menu_item() {
  auto list_view = static_cast<ListView*>(m_context_menu->layout()->itemAt(0)->widget());
  auto export_item = list_view->get_list_item(list_view->get_list()->get_size() - 1);
  if(m_table_view->get_table()->get_row_size() == 0) {
    if(export_item->isEnabled()) {
      export_item->setEnabled(false);
      update_style(*export_item, [] (auto& style) {
        style.get(Any() > is_a<TextBox>()).set(TextColor(QColor(0xC8C8C8)));
      });
    }
  } else {
    if(!export_item->isEnabled()) {
      export_item->setEnabled(true);
      update_style(*export_item, [] (auto& style) {
        style.get(Any() > is_a<TextBox>()).set(TextColor(Qt::black));
      });
    }
  }
}

void TimeAndSalesWindow::on_current(const Security& security) {
  auto prefix_name = displayText(security) + " " + QString(0x2013) + " ";
  m_title_label->get_labels()->set(0, prefix_name + TITLE_NAME);
  m_title_label->get_labels()->set(1, prefix_name + TITLE_SHORT_NAME);
  setWindowTitle(m_title_label->get_current()->get());
  m_transition_view->set_status(TransitionView::Status::NONE);
  m_is_updating_model = true;
  m_table_view->get_table()->set_model(m_model_builder(security));
}

void TimeAndSalesWindow::on_table_operation(const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      auto time_and_sale_style = m_properties.get_style(m_table_view->get_table()->get(operation.m_index));
      for(auto column = 0; column < m_table_view->get_table()->get_column_size(); ++column) {
        auto item = m_table_view->get_item({operation.m_index, column});
        update_style(*item, [&] (auto& style) {
          style.get(Any() > is_a<TextBox>()).
            set(text_style(time_and_sale_style.m_font, QColor(time_and_sale_style.m_text_color)));
          style.get(Any() > Body()).
            set(BackgroundColor(time_and_sale_style.m_band_color)).
            set(horizontal_padding(scale_width(2))).
            set(vertical_padding(scale_height(1.5)));
        });
        item->setDisabled(true);
      }
      update_export_menu_item();
    },
    [&] (const TableModel::RemoveOperation& operation) {
      update_export_menu_item();
    });
}
