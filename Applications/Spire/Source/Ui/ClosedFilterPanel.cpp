#include "Spire/Ui/ClosedFilterPanel.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QEvent>
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/FilterPanel.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto LIST_ITEM_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border_size(0)).
      set(PaddingLeft(scale_width(8))).
      set(PaddingRight(scale_width(10))).
      set(vertical_padding(scale_height(5)));
    return style;
  }

  auto LIST_VIEW_STYLE(StyleSheet style) {
    style.get(Any()).
      set(EdgeNavigation::CONTAIN);
    return style;
  }

  void invalidate_descendants(QWidget& widget) {
    for(auto child : widget.children()) {
      if(!child->isWidgetType()) {
        continue;
      }
      auto& widget = *static_cast<QWidget*>(child);
      invalidate_descendants(widget);
      widget.updateGeometry();
      if(widget.layout()) {
        widget.layout()->invalidate();
      }
    }
  }
}

ClosedFilterPanel::ClosedFilterPanel(std::shared_ptr<TableModel> model,
    QString title, QWidget& parent)
    : QWidget(&parent),
      m_model(std::move(model)),
      m_list_model(std::make_shared<ArrayListModel>()),
      m_submission(std::make_shared<ArrayListModel>()),
      m_model_connection(m_model->connect_operation_signal(
        std::bind_front(&ClosedFilterPanel::on_table_model_operation, this))) {
  for(auto i = 0; i < m_model->get_row_size(); ++i) {
    add_item(i);
    if(m_model->get<bool>(i, 1)) {
      m_submission->push(m_model->at(i, 0));
    }
  }
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_list_view = new ListView(m_list_model, [=] (const auto& model, int index) {
    auto& item = model->get<std::shared_ptr<Item>>(index);
    auto check_box = new CheckBox(item->m_model);
    check_box->set_label(displayTextAny(m_model->at(item->m_index, 0)));
    check_box->setLayoutDirection(Qt::RightToLeft);
    check_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return check_box;
  });
  set_style(*m_list_view, LIST_VIEW_STYLE(get_style(*m_list_view)));
  for(auto i = 0; i < m_list_model->get_size(); ++i) {
    set_style(*m_list_view->get_list_item(i), LIST_ITEM_STYLE());
  }
  m_list_model->connect_operation_signal(
    std::bind_front(&ClosedFilterPanel::on_list_model_operation, this));
  auto scrollable_list_box = new ScrollableListBox(*m_list_view);
  scrollable_list_box->setMinimumSize(scale_width(160), scale_height(54));
  scrollable_list_box->setMaximumHeight(scale_height(158));
  if(scrollable_list_box->sizeHint().height() >
      scrollable_list_box->maximumHeight()) {
    scrollable_list_box->get_scroll_box().get_vertical_scroll_bar().show();
  }
  layout->addWidget(scrollable_list_box);
  m_filter_panel = new FilterPanel(std::move(title), this, parent);
  m_filter_panel->connect_reset_signal(
    std::bind_front(&ClosedFilterPanel::on_reset, this));
  window()->installEventFilter(this);
}

const std::shared_ptr<TableModel>& ClosedFilterPanel::get_model() const {
  return m_model;
}

connection ClosedFilterPanel::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool ClosedFilterPanel::eventFilter(QObject* watched, QEvent* event) {
  if(window() == watched && event->type() == QEvent::Close) {
    m_filter_panel->hide();
    hide();
  }
  return QWidget::eventFilter(watched, event);
}

bool ClosedFilterPanel::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_filter_panel->show();
  } else if(event->type() == QEvent::HideToParent) {
    m_filter_panel->hide();
  }
  return QWidget::event(event);
}

void ClosedFilterPanel::add_item(int index) {
  auto boolean_model =
    std::make_shared<LocalBooleanModel>(m_model->get<bool>(index, 1));
  m_list_model->insert(std::make_shared<Item>(boolean_model, index), index);
  boolean_model->connect_current_signal(
    std::bind_front(&ClosedFilterPanel::on_current, this,
      m_list_model->get<std::shared_ptr<Item>>(index)));
}

void ClosedFilterPanel::clear_submission() {
  auto index = m_submission->get_size();
  while(--index >= 0) {
    m_submission->remove(index);
  }
}

void ClosedFilterPanel::update_submission(int index, bool is_checked) {
  auto submission_index = [=] {
    auto value = displayTextAny(m_model->at(index, 0));
    for(auto i = 0; i < m_submission->get_size(); ++i) {
      if(value == displayTextAny(m_submission->at(i))) {
        return i;
      }
    }
    return -1;
  }();
  if(submission_index >= 0 && !is_checked) {
    m_submission->remove(submission_index);
  } else if(submission_index == -1 && is_checked) {
    m_submission->push(m_model->at(index, 0));
  }
}

void ClosedFilterPanel::on_current(const std::shared_ptr<Item>& item,
    bool is_checked) {
  if(m_model->get<bool>(item->m_index, 1) != is_checked) {
    auto blocker = shared_connection_block(m_model_connection);
    m_model->set(item->m_index, 1, is_checked);
    update_submission(item->m_index, is_checked);
    m_submit_signal(m_submission);
  }
}

void ClosedFilterPanel::on_list_model_operation(
    const ListModel::Operation& operation) {
  visit(operation,
    [=] (const ListModel::AddOperation& operation) {
      set_style(*m_list_view->get_list_item(operation.m_index),
        LIST_ITEM_STYLE());
      invalidate_descendants(*window());
    },
    [=] (const ListModel::RemoveOperation& operation) {
      invalidate_descendants(*window());
    });
}

void ClosedFilterPanel::on_table_model_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [=] (const TableModel::UpdateOperation& operation) {
      if(operation.m_column == 1) {
        auto is_checked = m_model->get<bool>(operation.m_row, 1);
        m_list_model->get<std::shared_ptr<Item>>(operation.m_row)->
          m_model->set_current(is_checked);
        update_submission(operation.m_row, is_checked);
      }
    },
    [=] (const TableModel::AddOperation& operation) {
      add_item(operation.m_index);
      for(auto i = operation.m_index + 1; i < m_list_model->get_size(); ++i) {
        ++(m_list_model->get<std::shared_ptr<Item>>(i)->m_index);
      }
      if(m_model->get<bool>(operation.m_index, 1)) {
        m_submission->push(m_model->at(operation.m_index, 0));
      }
    },
    [=] (const TableModel::RemoveOperation& operation) {
      m_list_model->remove(operation.m_index);
      for(auto i = operation.m_index; i < m_list_model->get_size(); ++i) {
        --(m_list_model->get<std::shared_ptr<Item>>(i)->m_index);
      }
      clear_submission();
      for(auto i = 0; i < m_model->get_row_size(); ++i) {
        if(m_model->get<bool>(i, 1)) {
          m_submission->push(m_model->at(i, 0));
        }
      }
    });
}

void ClosedFilterPanel::on_reset() {
  clear_submission();
  for(auto i = 0; i < m_model->get_row_size(); ++i) {
    m_model->set(i, 1, true);
  }
  m_submit_signal(m_submission);
}
