#include "Spire/Ui/ClosedFilterPanel.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QEvent>
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CheckBox.hpp"
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
  struct ItemEntry {
    std::any m_data;
    std::shared_ptr<BooleanModel> m_model;
    int m_index;
  };

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

  auto make_check_box(const std::shared_ptr<ListModel>& model, int index) {
    auto item = model->get<ItemEntry>(index);
    auto check_box = new CheckBox(item.m_model);
    check_box->set_label(displayTextAny(item.m_data));
    check_box->setLayoutDirection(Qt::RightToLeft);
    check_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return check_box;
  }

  void invalidate_children_recursively(QWidget& widget) {
    for(auto child : widget.children()) {
      if(!child->isWidgetType()) {
        continue;
      }
      if(auto w = qobject_cast<QWidget*>(child)) {
        invalidate_children_recursively(*w);
        w->updateGeometry();
        if(w->layout()) {
          w->layout()->invalidate();
        }
      }
    }
  }
}

class ClosedFilterPanelModelAdaptor : public ListModel {
  public:
    ClosedFilterPanelModelAdaptor(std::shared_ptr<TableModel> source)
        : m_source(std::move(source)),
          m_source_connection(m_source->connect_operation_signal(
            std::bind_front(
              &ClosedFilterPanelModelAdaptor::on_operation, this))) {
      for(auto i = 0; i < m_source->get_row_size(); ++i) {
        auto boolean_model =
          std::make_shared<LocalBooleanModel>(m_source->get<bool>(i, 1));
        m_data.emplace_back(ItemEntry{m_source->at(i, 0), boolean_model, i});
        boolean_model->connect_current_signal(
          std::bind_front(&ClosedFilterPanelModelAdaptor::on_current, this,
            &std::any_cast<ItemEntry&>(m_data.back())));
      }
    }

    int get_size() const override {
      return m_source->get_row_size();
    }

    const std::any& at(int index) const override {
      if(index < 0 || index >= get_size()) {
        throw std::out_of_range("The index is out of range.");
      }
      return m_data[index];
    }

    connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override {
      return m_transaction.connect_operation_signal(slot);
    }

    void on_operation(const TableModel::Operation& operation) {
      m_transaction.transact([&] {
        visit(operation,
          [&] (const TableModel::AddOperation& operation) {
            add_item(operation.m_index);
          },
          [&] (const TableModel::RemoveOperation& operation) {
            remove_item(operation.m_index);
          },
          [&] (const TableModel::UpdateOperation& operation) {
            if(operation.m_column == 1) {
              std::any_cast<ItemEntry&>(m_data[operation.m_row]).m_model->
                set_current(m_source->get<bool>(operation.m_row, 1));
            }
          });
        });
    }

    void on_current(ItemEntry* item, bool is_checked) {
      auto blocker = shared_connection_block(m_source_connection);
      m_source->set(item->m_index, 1, is_checked);
      m_transaction.push(UpdateOperation{item->m_index});
    }

    void add_item(int index) {
      auto boolean_model =
        std::make_shared<LocalBooleanModel>(m_source->get<bool>(index, 1));
      m_data.emplace(m_data.begin() + index,
        ItemEntry{m_source->at(index, 0), boolean_model, index});
      boolean_model->connect_current_signal(
        std::bind_front(&ClosedFilterPanelModelAdaptor::on_current, this,
          &std::any_cast<ItemEntry&>(m_data[index])));
      for(auto i = m_data.begin() + index + 1; i != m_data.end(); ++i) {
        ++(std::any_cast<ItemEntry&>(*i).m_index);
      }
      m_transaction.push(AddOperation{index});
    }

    void remove_item(int index) {
      m_data.erase(m_data.begin() + index);
      for(auto i = m_data.begin() + index; i != m_data.end(); ++i) {
        --(std::any_cast<ItemEntry&>(*i).m_index);
      }
      m_transaction.push(RemoveOperation{index});
    }

  private:
    std::shared_ptr<TableModel> m_source;
    std::vector<std::any> m_data;
    ListModelTransactionLog m_transaction;
    scoped_connection m_source_connection;
};

ClosedFilterPanel::ClosedFilterPanel(std::shared_ptr<TableModel> model,
    QString title, QWidget& parent)
    : QWidget(&parent),
      m_model(std::move(model)),
      m_submission(std::make_shared<ArrayListModel>()),
      m_model_connection(m_model->connect_operation_signal(
        std::bind_front(&ClosedFilterPanel::on_table_model_operation, this))) {
  for(auto i = 0; i < m_model->get_row_size(); ++i) {
    if(m_model->get<bool>(i, 1)) {
      m_submission->push(m_model->at(i, 0));
    }
  }
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_list_view =
    new ListView(std::make_shared<ClosedFilterPanelModelAdaptor>(m_model),
      make_check_box);
  set_style(*m_list_view, LIST_VIEW_STYLE(get_style(*m_list_view)));
  for(auto i = 0; i < m_list_view->get_list_model()->get_size(); ++i) {
    set_style(*m_list_view->get_list_item(i), LIST_ITEM_STYLE());
  }
  m_list_view->get_list_model()->connect_operation_signal(
    std::bind_front(&ClosedFilterPanel::on_list_model_operation, this));
  m_scrollable_list_box = new ScrollableListBox(*m_list_view);
  m_scrollable_list_box->setMinimumSize(scale_width(160), scale_height(54));
  m_scrollable_list_box->setMaximumHeight(scale_height(158));
  if(m_scrollable_list_box->sizeHint().height() >
      m_scrollable_list_box->maximumHeight()) {
    m_scrollable_list_box->get_scroll_box().get_vertical_scroll_bar().show();
  }
  layout->addWidget(m_scrollable_list_box);
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

void ClosedFilterPanel::on_reset() {
  for(auto i = 0; i < m_model->get_row_size(); ++i) {
    m_model->set(i, 1, true);
  }
}

void ClosedFilterPanel::on_table_model_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [=] (const TableModel::UpdateOperation& operation) {
      auto value = displayTextAny(m_model->at(operation.m_row, 0));
      auto i = 0;
      for(i = 0; i < m_submission->get_size(); ++i) {
        if(value == displayTextAny(m_submission->at(i))) {
          break;
        }
      }
      if(i < m_submission->get_size()) {
        if(!m_model->get<bool>(operation.m_row, 1)) {
          m_submission->remove(i);
        }
      } else {
        if(m_model->get<bool>(operation.m_row, 1)) {
          m_submission->push(m_model->at(operation.m_row, 0));
        }
      }
      m_submit_signal(m_submission);
    },
    [=] (const TableModel::AddOperation& operation) {
      if(m_model->get<bool>(operation.m_index, 1)) {
        m_submission->push(m_model->at(operation.m_index, 0));
      }
    },
    [=] (const TableModel::RemoveOperation& operation) {
      auto index = m_submission->get_size();
      while(--index >= 0) {
        m_submission->remove(index);
      }
      for(auto i = 0; i < m_model->get_row_size(); ++i) {
        if(m_model->get<bool>(i, 1)) {
          m_submission->push(m_model->at(i, 0));
        }
      }
    });
}

void ClosedFilterPanel::on_list_model_operation(
    const ListModel::Operation& operation) {
  visit(operation,
    [=] (const ListModel::AddOperation& operation) {
      set_style(*m_list_view->get_list_item(operation.m_index),
        LIST_ITEM_STYLE());
      invalidate_children_recursively(*window());
    },
    [=] (const ListModel::RemoveOperation& operation) {
      invalidate_children_recursively(*window());
    });
}
