#include "Spire/Ui/ClosedFilterPanel.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QCoreApplication>
#include <QEvent>
#include "Spire/Spire/ColumnViewListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/FilterPanel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"
#include "Spire/Ui/ScrollBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto LIST_VIEW_STYLE(StyleSheet style) {
    style.get(Any()).
      set(Qt::Horizontal).
      set(EdgeNavigation::CONTAIN).
      set(ListItemGap(scale_width(18)));
    style.get(Any() > is_a<ListItem>()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border_size(0)).
      set(padding(0));
    return style;
  }
}

ClosedFilterPanel::ClosedFilterPanel(std::shared_ptr<TableModel> table,
    QWidget* parent)
    : QWidget(parent),
      m_table(std::move(table)),
      m_submission(std::make_shared<ArrayListModel<std::any>>()),
      m_table_connection(m_table->connect_operation_signal(
        std::bind_front(&ClosedFilterPanel::on_table_model_operation, this))) {
  for(auto i = 0; i < m_table->get_row_size(); ++i) {
    if(m_table->get<bool>(i, 1)) {
      m_submission->push(to_any(m_table->at(i, 0)));
    }
  }
  auto list_view = new ListView(
    std::make_shared<ColumnViewListModel<bool>>(m_table, 1),
    [=] (const std::shared_ptr<ListModel<bool>>& list, int index) {
      auto check_box = new CheckBox(make_list_value_model(list, index));
      check_box->set_label(to_text(m_table->at(index, 0)));
      check_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      check_box->focusProxy()->installEventFilter(this);
      return check_box;
    });
  update_style(*list_view, [] (auto& style) {
    style = LIST_VIEW_STYLE(style);
  });
  list_view->get_list()->connect_operation_signal(
    std::bind_front(&ClosedFilterPanel::on_list_model_operation, this));
  auto scrollable_list_box = new ScrollableListBox(*list_view);
  scrollable_list_box->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Fixed);
  scrollable_list_box->get_scroll_box().set(ScrollBox::DisplayPolicy::NEVER);
  update_style(*scrollable_list_box, [] (auto& style) {
    style.get(Any()).
      set(border_size(0)).
      set(PaddingLeft(scale_width(8)));
  });
  auto filter_panel = new FilterPanel(*scrollable_list_box);
  filter_panel->connect_reset_signal(
    std::bind_front(&ClosedFilterPanel::on_reset, this));
  update_style(*filter_panel, [] (auto& style) {
    style.get(Any()).set(PaddingLeft(0));
  });
  enclose(*this, *filter_panel);
  proxy_style(*this, *filter_panel);
  list_view->get_current()->set(0);
}

const std::shared_ptr<TableModel>& ClosedFilterPanel::get_table() const {
  return m_table;
}

connection ClosedFilterPanel::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool ClosedFilterPanel::eventFilter(QObject* object, QEvent* event) {
  if(event->type() == QEvent::MouseButtonPress ||
      event->type() == QEvent::MouseButtonDblClick ||
      event->type() == QEvent::MouseButtonRelease) {
    QCoreApplication::sendEvent(object->parent(), event);
  }
  return QWidget::eventFilter(object, event);
}

void ClosedFilterPanel::on_list_model_operation(
    const AnyListModel::Operation& operation) {
  visit(operation,
    [&] (const AnyListModel::AddOperation& operation) {
      if(m_table->get<bool>(operation.m_index, 1)) {
        m_submission->push(to_any(m_table->at(operation.m_index, 0)));
      }
    },
    [&] (const AnyListModel::RemoveOperation& operation) {
      clear(*m_submission);
      for(auto i = 0; i < m_table->get_row_size(); ++i) {
        if(m_table->get<bool>(i, 1)) {
          m_submission->push(to_any(m_table->at(i, 0)));
        }
      }
    });
}

void ClosedFilterPanel::on_table_model_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::UpdateOperation& operation) {
      auto index = [&] {
        auto value = to_text(m_table->at(operation.m_row, 0));
        for(auto i = 0; i < m_submission->get_size(); ++i) {
          if(value == to_text(m_submission->get(i))) {
            return i;
          }
        }
        return -1;
      }();
      if(index >= 0 && !m_table->get<bool>(operation.m_row, 1)) {
        m_submission->remove(index);
        m_submit_signal(m_submission);
      } else if(index == -1 && m_table->get<bool>(operation.m_row, 1)) {
        m_submission->push(to_any(m_table->at(operation.m_row, 0)));
        m_submit_signal(m_submission);
      }
    });
}

void ClosedFilterPanel::on_reset() {
  auto blocker = shared_connection_block(m_table_connection);
  for(auto i = 0; i < m_table->get_row_size(); ++i) {
    if(!m_table->get<bool>(i, 1)) {
      m_table->set(i, 1, true);
      m_submission->push(to_any(m_table->at(i, 0)));
    }
  }
  m_submit_signal(m_submission);
}
