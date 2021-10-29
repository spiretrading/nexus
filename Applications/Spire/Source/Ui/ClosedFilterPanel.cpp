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

class BooleanModelAdptor : public BooleanModel {
  public:
    BooleanModelAdptor(std::shared_ptr<TableModel> source, int row)
      : m_source(std::move(source)),
        m_row(row),
        m_source_connection(m_source->connect_operation_signal(
          std::bind_front(&BooleanModelAdptor::on_operation, this))) {}

    int get_row() const {
      return m_row;
    }

    void set_row(int row) {
      m_row = row;
    }

    const bool& get_current() const override {
      return m_source->get<bool>(m_row, 1);
    }

    QValidator::State set_current(const bool& value) override {
      m_source->set(m_row, 1, value);
      return QValidator::State::Acceptable;
    }

    connection connect_current_signal(
        const typename CurrentSignal::slot_type& slot) const override {
      return m_current_signal.connect(slot);
    }

    void on_operation(const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::UpdateOperation& operation) {
          if(operation.m_row == m_row && operation.m_column == 1) {
            m_current_signal(m_source->get<bool>(m_row, 1));
          }
        });
    }

  private:
    mutable CurrentSignal m_current_signal;
    std::shared_ptr<TableModel> m_source;
    int m_row;
    scoped_connection m_source_connection;
};

class TableModelToListModel : public ArrayListModel {
  public:
    explicit TableModelToListModel(std::shared_ptr<TableModel> source)
        : m_source(std::move(source)),
          m_source_connection(m_source->connect_operation_signal(
            std::bind_front(&TableModelToListModel::on_operation, this))) {
      for(auto i = 0; i < m_source->get_row_size(); ++i) {
        push(std::make_shared<BooleanModelAdptor>(m_source, i));
      }
    }

    void on_operation(const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::AddOperation& operation) {
          insert(std::make_shared<BooleanModelAdptor>(m_source,
            operation.m_index), operation.m_index);
          for(auto i = operation.m_index + 1; i < get_size(); ++i) {
            auto& boolean_model = get<std::shared_ptr<BooleanModelAdptor>>(i);
            boolean_model->set_row(boolean_model->get_row() + 1);
          }
        },
        [&] (const TableModel::RemoveOperation& operation) {
          remove(operation.m_index);
          for(auto i = operation.m_index; i < get_size(); ++i) {
            auto& boolean_model = get<std::shared_ptr<BooleanModelAdptor>>(i);
            boolean_model->set_row(boolean_model->get_row() - 1);
          }
        });
    }

  private:
    std::shared_ptr<TableModel> m_source;
    scoped_connection m_source_connection;
};

ClosedFilterPanel::ClosedFilterPanel(std::shared_ptr<TableModel> model,
    QString title, QWidget& parent)
    : QWidget(&parent),
      m_model(std::move(model)) {
  m_list_view = new ListView(
    std::make_shared<TableModelToListModel>(m_model),
    [=] (const auto& model, int index) {
      auto& boolean_model =
        model->get<std::shared_ptr<BooleanModelAdptor>>(index);
      boolean_model->connect_current_signal(
        std::bind_front(&ClosedFilterPanel::on_current, this));
      auto check_box = new CheckBox(boolean_model);
      check_box->set_label(displayTextAny(m_model->at(index, 0)));
      check_box->setLayoutDirection(Qt::RightToLeft);
      check_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
      return check_box;
    });
  set_style(*m_list_view, LIST_VIEW_STYLE(get_style(*m_list_view)));
  for(auto i = 0; i < m_list_view->get_list_model()->get_size(); ++i) {
    set_style(*m_list_view->get_list_item(i), LIST_ITEM_STYLE());
  }
  m_list_view->get_list_model()->connect_operation_signal(
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

void ClosedFilterPanel::on_current(bool is_checked) {
  auto submission = std::make_shared<ArrayListModel>();
  for(auto i = 0; i < m_model->get_row_size(); ++i) {
    if(m_model->get<bool>(i, 1)) {
      submission->push(m_model->at(i, 0));
    }
  }
  m_submit_signal(submission);
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

void ClosedFilterPanel::on_reset() {
  for(auto i = 0; i < m_model->get_row_size(); ++i) {
    if(!m_model->get<bool>(i, 1)) {
      m_model->set(i, 1, true);
    }
  }
}
