#include "Spire/Ui/ClosedFilterPanel.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QEvent>
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/ColumnViewListModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/FilterPanel.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListValueModel.hpp"
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

class BooleanListValueModel : public BooleanModel {
  public:
    using Type = BooleanModel::Type;

    explicit BooleanListValueModel(std::shared_ptr<ListValueModel> source)
      : m_source(std::move(source)),
        m_source_connection(m_source->connect_current_signal(
          std::bind_front(&BooleanListValueModel::on_current, this))) {}

    const Type& get() const override {
      return std::any_cast<const Type&>(m_source->get());
    }

    QValidator::State set(const Type& value) override {
      return m_source->set(value);
    }

    connection connect_current_signal(
        const typename UpdateSignal::slot_type& slot) const override {
      return m_current_signal.connect(slot);
    }

  private:
    mutable UpdateSignal m_current_signal;
    std::shared_ptr<ListValueModel> m_source;
    scoped_connection m_source_connection;

    void on_current(const std::any& current) {
      m_current_signal(std::any_cast<const Type&>(current));
    }
};

ClosedFilterPanel::ClosedFilterPanel(std::shared_ptr<TableModel> table,
    QString title, QWidget& parent)
    : QWidget(&parent),
      m_table(std::move(table)),
      m_submission(std::make_shared<ArrayListModel>()),
      m_table_connection(m_table->connect_operation_signal(
        std::bind_front(&ClosedFilterPanel::on_table_model_operation, this))) {
  for(auto i = 0; i < m_table->get_row_size(); ++i) {
    if(m_table->get<bool>(i, 1)) {
      m_submission->push(m_table->at(i, 0));
    }
  }
  m_list_view = new ListView(std::make_shared<ColumnViewListModel>(m_table, 1),
    [=] (const auto& table, int index) {
      auto check_box = new CheckBox(std::make_shared<BooleanListValueModel>(
        std::make_shared<ListValueModel>(table, index)));
      check_box->set_label(displayTextAny(m_table->at(index, 0)));
      check_box->setLayoutDirection(Qt::RightToLeft);
      check_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
      return check_box;
    });
  update_style(*m_list_view, [&] (auto& style) {
    style = LIST_VIEW_STYLE(style);
  });
  for(auto i = 0; i < m_list_view->get_list()->get_size(); ++i) {
    set_style(*m_list_view->get_list_item(i), LIST_ITEM_STYLE());
  }
  m_list_view->get_current()->set(0);
  m_list_view->get_list()->connect_operation_signal(
    std::bind_front(&ClosedFilterPanel::on_list_model_operation, this));
  auto scrollable_list_box = new ScrollableListBox(*m_list_view);
  scrollable_list_box->setMinimumSize(scale_width(160), scale_height(54));
  scrollable_list_box->setMaximumHeight(scale_height(158));
  if(scrollable_list_box->sizeHint().height() >
      scrollable_list_box->maximumHeight()) {
    scrollable_list_box->get_scroll_box().get_vertical_scroll_bar().show();
  }
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(scrollable_list_box);
  setFocusProxy(scrollable_list_box);
  m_filter_panel = new FilterPanel(std::move(title), this, parent);
  m_filter_panel->connect_reset_signal(
    std::bind_front(&ClosedFilterPanel::on_reset, this));
  window()->installEventFilter(this);
}

const std::shared_ptr<TableModel>& ClosedFilterPanel::get_table() const {
  return m_table;
}

connection ClosedFilterPanel::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool ClosedFilterPanel::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Close) {
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

void ClosedFilterPanel::on_list_model_operation(
    const ListModel::Operation& operation) {
  visit(operation,
    [&] (const ListModel::AddOperation& operation) {
      set_style(
        *m_list_view->get_list_item(operation.m_index), LIST_ITEM_STYLE());
      invalidate_descendants(*window());
      if(m_table->get<bool>(operation.m_index, 1)) {
        m_submission->push(m_table->at(operation.m_index, 0));
      }
    },
    [&] (const ListModel::RemoveOperation& operation) {
      invalidate_descendants(*window());
      auto index = m_submission->get_size();
      while(--index >= 0) {
        m_submission->remove(index);
      }
      for(auto i = 0; i < m_table->get_row_size(); ++i) {
        if(m_table->get<bool>(i, 1)) {
          m_submission->push(m_table->at(i, 0));
        }
      }
    });
}

void ClosedFilterPanel::on_table_model_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::UpdateOperation& operation) {
      auto index = [&] {
        auto value = displayTextAny(m_table->at(operation.m_row, 0));
        for(auto i = 0; i < m_submission->get_size(); ++i) {
          if(value == displayTextAny(m_submission->at(i))) {
            return i;
          }
        }
        return -1;
      }();
      if(index >= 0 && !m_table->get<bool>(operation.m_row, 1)) {
        m_submission->remove(index);
        m_submit_signal(m_submission);
      } else if(index == -1 && m_table->get<bool>(operation.m_row, 1)) {
        m_submission->push(m_table->at(operation.m_row, 0));
        m_submit_signal(m_submission);
      }
    });
}

void ClosedFilterPanel::on_reset() {
  auto blocker = shared_connection_block(m_table_connection);
  for(auto i = 0; i < m_table->get_row_size(); ++i) {
    if(!m_table->get<bool>(i, 1)) {
      m_table->set(i, 1, true);
      m_submission->push(m_table->at(i, 0));
    }
  }
  m_submit_signal(m_submission);
}
