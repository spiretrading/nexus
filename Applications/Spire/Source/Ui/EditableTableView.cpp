#include "Spire/Ui/EditableTableView.hpp"
#include <QApplication>
#include <QKeyEvent>
#include <QTimer>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/TableRowIndexTracker.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/KeyInputBox.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using DeleteButton = StateSelector<void, struct DeleteButtonSeletorTag>;
  using EmptyCell = StateSelector<void, struct EmptyCellSeletorTag>;

  auto TABLE_VIEW_STYLE() {
    auto style = StyleSheet();
    auto body_selector = Any() > is_a<TableBody>();
    auto item_selector = body_selector > Row() > is_a<TableItem>();
    style.get(item_selector > Any() >
        (ReadOnly() && !(+Any() << is_a<ListItem>()) && !Prompt())).
      set(horizontal_padding(scale_width(8)));
    style.get(item_selector >  Any() > ReadOnly() >
        (is_a<TextBox>() && !(+Any() << is_a<ListItem>()) && !Prompt())).
      set(horizontal_padding(scale_width(8)));
    style.get((item_selector > !ReadOnly()) << Current()).
      set(border_color(QColor(Qt::transparent)));
    style.get(body_selector > Row() > Current()).
      set(BackgroundColor(Qt::transparent));
    style.get(body_selector > Row() > HoverItem()).
      set(border_color(QColor(0xA0A0A0)));
    style.get(body_selector > (Row() && Hover())).
      set(BackgroundColor(0xF2F2FF));
    style.get(item_selector > DeleteButton()).
      set(Visibility(Visibility::INVISIBLE));
    style.get(item_selector > DeleteButton() > is_a<Box>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(horizontal_padding(scale_width(2))).
      set(vertical_padding(scale_height(2)));
    style.get(item_selector > DeleteButton() > is_a<Icon>()).
      set(BackgroundColor(QColor(Qt::transparent)));
    style.get(body_selector > (CurrentRow() || (Row() && Hover())) >
        is_a<TableItem>() > DeleteButton()).
      set(Visibility(Visibility::VISIBLE));
    style.get((body_selector > (CurrentRow() || (Row() && Hover()))) >
        DeleteButton() > is_a<Icon>()).
      set(Fill(QColor(0x535353)));
    style.get(body_selector > (Row() && Hover()) > DeleteButton() >
        (is_a<Icon>() && Hover())).
      set(BackgroundColor(QColor(0xDFDFEB))).
      set(Fill(QColor(0xB71C1C)));
    style.get(body_selector > CurrentRow() > DeleteButton() >
        (is_a<Icon>() && Hover())).
      set(BackgroundColor(QColor(0xD0CEEB))).
      set(Fill(QColor(0xB71C1C)));
    style.get((item_selector > EmptyCell()) << (HoverItem() || Current())).
      set(border_color(QColor(Qt::transparent)));
    style.get(body_selector > CurrentRow()).
      set(BackgroundColor(QColor(0xE2E0FF)));
    style.get(body_selector > CurrentColumn()).
      set(BackgroundColor(Qt::transparent));
    return style;
  }

  QWidget* make_empty_cell() {
    auto cell = new QWidget();
    cell->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    match(*cell, EmptyCell());
    return cell;
  }

  struct Tracker {
    TableRowIndexTracker m_index;
    scoped_connection m_connection;

    Tracker(int index)
      : m_index(index) {}
  };

  struct EditableTableCurrentModel : EditableTableView::CurrentModel {
    mutable UpdateSignal m_update_signal;
    std::shared_ptr<EditableTableView::CurrentModel> m_current;
    int m_columns;
    optional<TableIndex> m_value;
    scoped_connection m_current_connection;

    EditableTableCurrentModel(
        std::shared_ptr<EditableTableView::CurrentModel> current, int columns)
        : m_current(std::move(current)),
          m_columns(columns),
          m_current_connection(m_current->connect_update_signal(
            std::bind_front(&EditableTableCurrentModel::on_update, this))) {
      auto value = m_current->get();
      if(test(value) != QValidator::State::Invalid) {
        m_value = std::move(value);
      }
    }

    QValidator::State get_state() const override {
      return m_current->get_state();
    }

    const Type& get() const override {
      return m_value;
    }

    QValidator::State test(const Type& value) const override {
      if(value && value->m_column == m_columns - 1) {
        return QValidator::State::Invalid;
      }
      return m_current->test(value);
    }

    QValidator::State set(const Type& value) override {
      if(test(value) == QValidator::State::Invalid) {
        return QValidator::State::Invalid;
      }
      return m_current->set(value);
    }

    connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const {
      return m_update_signal.connect(slot);
    }

    void on_update(const optional<TableIndex>& index) {
      if(test(index) == QValidator::State::Invalid) {
        m_value = none;
        m_update_signal(none);
      } else {
        m_value = index;
        m_update_signal(index);
      }
    }
  };

  struct EditableTableModel : TableModel {
    std::shared_ptr<TableModel> m_source;
    std::shared_ptr<EditableTableView::HeaderModel> m_header;
    TableModelTransactionLog m_transaction;
    scoped_connection m_source_connection;

    explicit EditableTableModel(std::shared_ptr<TableModel> source,
      std::shared_ptr<EditableTableView::HeaderModel> header)
      : m_source(std::move(source)),
        m_header(std::move(header)),
        m_source_connection(m_source->connect_operation_signal(
          std::bind_front(&EditableTableModel::on_operation, this))) {}

    int get_row_size() const override {
      return m_source->get_row_size();
    }

    int get_column_size() const override {
      return m_header->get_size() + 2;
    }

    AnyRef at(int row, int column) const override {
      if(row < 0 || row >= get_row_size() || column < 0 ||
          column >= get_column_size()) {
        throw std::out_of_range("The row or column is out of range.");
      }
      if(column == 0) {
        return AnyRef(row, AnyRef::by_value); 
      }
      column -= 1;
      if(row < m_source->get_row_size() &&
          column < m_source->get_column_size()) {
        return m_source->at(row, column);
      }
      return {};
    }

    QValidator::State set(int row, int column, const std::any& value) override {
      if(row < 0 || row >= get_row_size() || column < 0 ||
          column >= get_column_size()) {
        throw std::out_of_range("The row or column is out of range.");
      }
      if(column == 0) {
        return QValidator::State::Invalid;
      }
      column -= 1;
      if(row < m_source->get_row_size() &&
          column < m_source->get_column_size()) {
        return m_source->set(row, column, value);
      }
      return QValidator::State::Invalid;
    }

    QValidator::State remove(int row) override {
      return m_source->remove(row);
    }

    connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override {
      return m_transaction.connect_operation_signal(slot);
    }

    void on_operation(const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::UpdateOperation& operation) {
          m_transaction.push(TableModel::UpdateOperation(operation.m_row,
            operation.m_column + 1, operation.m_previous, operation.m_value));
        },
        [&] (const auto& operation) {
          m_transaction.push(operation);
        });
    }
  };

  struct EditableTableHeaderModel : ListModel<TableHeaderItem::Model> {
    std::shared_ptr<ListModel<TableHeaderItem::Model>> m_source;
    ListModelTransactionLog<TableHeaderItem::Model> m_transaction;
    scoped_connection m_source_connection;

    explicit EditableTableHeaderModel(
      std::shared_ptr<ListModel<TableHeaderItem::Model>> source)
      : m_source(std::move(source)),
        m_source_connection(m_source->connect_operation_signal(
          std::bind_front(&EditableTableHeaderModel::on_operation, this))) {}

    int get_size() const override {
      return m_source->get_size() + 2;
    }

    const TableHeaderItem::Model& get(int index) const override {
      if(index == 0 || index == get_size() - 1) {
        static auto model = TableHeaderItem::Model{"", "",
          TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE};
        return model;
      }
      return m_source->get(index - 1);
    }

    QValidator::State set(int index,
        const TableHeaderItem::Model& value) override {
      if(index == 0 || index == get_size() - 1) {
        return QValidator::Invalid;
      }
      return m_source->set(index - 1, value);
    }

    QValidator::State insert(const TableHeaderItem::Model& value,
        int index) override {
      return m_source->insert(value, index - 1);
    }

    QValidator::State move(int source, int destination) override {
      if(source == 0 || destination == 0 || source == get_size() - 1 ||
          destination == get_size() - 1) {
        return QValidator::Invalid;
      }
      return m_source->move(source - 1, destination - 1);
    }

    QValidator::State remove(int index) override {
      if(index == 0 || index == get_size() - 1) {
        return QValidator::Invalid;
      }
      return m_source->remove(index - 1);
    }

    connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override {
      return m_transaction.connect_operation_signal(slot);
    }

    void transact(const std::function<void ()>& transaction) override {
      m_transaction.transact(transaction);
    }

    void on_operation(const Operation& operation) {
      visit(operation,
        [&] (const AddOperation& operation) {
          m_transaction.push(AddOperation(operation.m_index + 1,
            operation.get_value()));
        },
        [&] (const MoveOperation& operation) {
          m_transaction.push(MoveOperation(operation.m_source + 1,
            operation.m_destination + 1));
        },
        [&] (const RemoveOperation& operation) {
          m_transaction.push(RemoveOperation(operation.m_index + 1,
            operation.get_value()));
        },
        [&] (const UpdateOperation& operation) {
          m_transaction.push(UpdateOperation(operation.m_index + 1,
            operation.get_previous(), operation.get_value()));
        });
    }
  };
}

EditableTableView::EditableTableView(
    std::shared_ptr<TableModel> table, std::shared_ptr<HeaderModel> header,
    std::shared_ptr<TableFilter> table_filter,
    std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection, ViewBuilder view_builder,
    Comparator comparator, QWidget* parent)
    : TableView(std::make_shared<EditableTableModel>(std::move(table), header),
        std::make_shared<EditableTableHeaderModel>(header),
        std::move(table_filter), std::make_shared<EditableTableCurrentModel>(
          std::move(current), header->get_size() + 2), std::move(selection),
        std::bind_front(
          &EditableTableView::make_table_item, this, std::move(view_builder)),
        std::move(comparator), parent),
      m_is_processing_key(false) {
  get_header().get_item(0)->set_is_resizeable(false);
  get_header().get_widths()->set(0, scale_width(26));
  set_style(*this, TABLE_VIEW_STYLE());
}

void EditableTableView::keyPressEvent(QKeyEvent* event) {
  if(auto& current = get_current()->get()) {
    if(m_is_processing_key) {
      return TableView::keyPressEvent(event);
    }
    m_is_processing_key = true;
    auto target = find_focus_proxy(get_body().get_item(*current)->get_body());
    QCoreApplication::sendEvent(target, event);
    target->setFocus();
    m_is_processing_key = false;
  } else {
    TableView::keyPressEvent(event);
  }
}

bool EditableTableView::focusNextPrevChild(bool next) {
  if(isEnabled()) {
    if(next) {
      if(navigate_next()) {
        return true;
      }
    } else if(navigate_previous()) {
      return true;
    }
  }
  auto next_focus_widget = static_cast<QWidget*>(this);
  auto next_widget = nextInFocusChain();
  while(next_widget && next_widget != this) {
    next_widget = next_widget->nextInFocusChain();
    if(!isAncestorOf(next_widget) && next_widget->isEnabled() &&
        next_widget->focusPolicy() & Qt::TabFocus) {
      next_focus_widget = next_widget;
      if(next) {
        break;
      }
    }
  }
  next_focus_widget->setFocus(Qt::TabFocusReason);
  return true;
}

QWidget* EditableTableView::make_table_item(const ViewBuilder& view_builder,
    const std::shared_ptr<TableModel>& table, int row, int column) {
  if(column == 0) {
    auto button = make_delete_icon_button();
    button->setMaximumHeight(scale_height(26));
    match(*button, DeleteButton());
    auto tracker = std::make_shared<Tracker>(row);
    tracker->m_connection = table->connect_operation_signal(
      std::bind_front(&TableRowIndexTracker::update, &tracker->m_index));
    button->connect_click_signal([=] {
      delete_row(tracker->m_index);
    });
    return button;
  } else if(column == table->get_column_size() - 1) {
    return make_empty_cell();
  } else {
    auto item = view_builder(
      std::static_pointer_cast<EditableTableModel>(get_table())->m_source,
      any_cast<int>(table->at(row, 0)), column - 1);
    item->connect_read_only_signal([=] (auto read_only) {
      if(read_only) {
        setFocus();
      }
    });
    return item;
  }
}

void EditableTableView::delete_row(const TableRowIndexTracker& row) {
  get_body().get_table()->remove(row.get_index());
}

bool EditableTableView::navigate_next() {
  if(auto& current = get_current()->get()) {
    auto column = current->m_column + 1;
    if(column >= get_table()->get_column_size() - 1) {
      auto row = current->m_row + 1;
      if(row >= get_table()->get_row_size()) {
        return false;
      } else {
        get_current()->set(Index(row, 0));
      }
    } else {
      get_current()->set(Index(current->m_row, column));
    }
  } else if(get_table()->get_row_size() > 0) {
    get_current()->set(Index(0, 0));
  } else {
    return false;
  }
  return true;
}

bool EditableTableView::navigate_previous() {
  if(auto& current = get_current()->get()) {
    auto column = current->m_column - 1;
    if(column < 0) {
      auto row = current->m_row - 1;
      if(row < 0) {
        return false;
      } else {
        get_current()->set(Index(row, get_table()->get_column_size() - 2));
      }
    } else {
      get_current()->set(Index(current->m_row, column));
    }
  } else if(get_table()->get_row_size() > 0) {
    get_current()->set(TableView::Index(
      get_table()->get_row_size() - 1, get_table()->get_column_size() - 2));
  } else {
    return false;
  }
  return true;
}
