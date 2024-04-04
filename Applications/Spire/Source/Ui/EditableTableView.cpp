#include "Spire/Ui/EditableTableView.hpp"
#include <QKeyEvent>
#include <QTimer>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/TableRowIndexTracker.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/Icon.hpp"
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
    style.get(Any() > is_a<TableItem>() >
        (ReadOnly() && !(+Any() << is_a<ListItem>()))).
      set(horizontal_padding(scale_width(8)));
    style.get(Any() > is_a<TableItem>() > ReadOnly() >
        (is_a<TextBox>() && !(+Any() << is_a<ListItem>()))).
      set(horizontal_padding(scale_width(8)));
    style.get(Any() > Current()).set(BackgroundColor(Qt::transparent));
    style.get(Any() > HoverItem()).set(border_color(QColor(0xA0A0A0)));
    style.get(Any() > (Row() && Hover())).
      set(BackgroundColor(0xF2F2FF));
    style.get(Any() > DeleteButton()).
      set(Visibility(Visibility::INVISIBLE));
    style.get(Any() > DeleteButton() > is_a<Box>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(horizontal_padding(scale_width(2))).
      set(vertical_padding(scale_height(2)));
    style.get(Any() > DeleteButton() > is_a<Icon>()).
      set(BackgroundColor(QColor(Qt::transparent)));
    style.get(Any() > (CurrentRow() || (Row() && Hover())) > DeleteButton()).
      set(Visibility(Visibility::VISIBLE));
    style.get((Any() > (CurrentRow() || (Row() && Hover()))) >
        DeleteButton() > is_a<Icon>()).
      set(Fill(QColor(0x535353)));
    style.get(Any() > (Row() && Hover()) > DeleteButton() >
        (is_a<Icon>() && Hover())).
      set(BackgroundColor(QColor(0xDFDFEB))).
      set(Fill(QColor(0xB71C1C)));
    style.get((Any() > DeleteButton()) << (HoverItem() || Current())).
      set(border_color(QColor(Qt::transparent)));
    style.get(Any() > CurrentRow() > DeleteButton() >
        (is_a<Icon>() && Hover())).
      set(BackgroundColor(QColor(0xD0CEEB))).
      set(Fill(QColor(0xB71C1C)));
    style.get((Any() > EmptyCell()) << (HoverItem() || Current())).
      set(border_color(QColor(Qt::transparent)));
    style.get(Any() > CurrentRow()).set(BackgroundColor(QColor(0xE2E0FF)));
    style.get(Any() > CurrentColumn()).set(BackgroundColor(Qt::transparent));
    return style;
  }

  QWidget* make_empty_cell() {
    auto cell = new QWidget();
    cell->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    match(*cell, EmptyCell());
    return cell;
  }

  EditableBox* find_editable_box(QWidget& widget) {
    if(auto editable_box = dynamic_cast<EditableBox*>(&widget)) {
      return editable_box;
    }
    if(auto layout = widget.layout(); layout && layout->count() == 1) {
      return find_editable_box(*layout->itemAt(0)->widget());
    }
    return nullptr;
  }

  struct Tracker {
    TableRowIndexTracker m_index;
    scoped_connection m_connection;

    Tracker(int index)
      : m_index(index) {}
  };
}

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
    if(row < m_source->get_row_size() && column < m_source->get_column_size()) {
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
    if(row < m_source->get_row_size() && column < m_source->get_column_size()) {
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
    auto adjust_row = [] (int index, const AnyListModel& source) {
      auto row = std::make_shared<ArrayListModel<std::any>>();
      row->push(index);
      for(auto i = 0; i < source.get_size(); ++i) {
        row->push(source.get(i));
      }
      row->push({});
      return row;
    };
    visit(operation,
      [&] (const TableModel::AddOperation& operation) {
        m_transaction.push(TableModel::AddOperation(operation.m_index,
          adjust_row(operation.m_index, *operation.m_row)));
      },
      [&] (const TableModel::MoveOperation& operation) {
        m_transaction.push(operation);
      },
      [&] (const TableModel::RemoveOperation& operation) {
        m_transaction.push(TableModel::RemoveOperation(operation.m_index,
          adjust_row(operation.m_index, *operation.m_row)));
      },
      [&] (const TableModel::UpdateOperation& operation) {
        m_transaction.push(TableModel::UpdateOperation(operation.m_row,
          operation.m_column + 1, operation.m_previous, operation.m_value));
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

EditableTableView::EditableTableView(
    std::shared_ptr<TableModel> table, std::shared_ptr<HeaderModel> header,
    std::shared_ptr<TableFilter> table_filter,
    std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection, ViewBuilder view_builder,
    Comparator comparator, QWidget* parent)
    : TableView(std::make_shared<EditableTableModel>(std::move(table), header),
        std::make_shared<EditableTableHeaderModel>(header),
        std::move(table_filter), std::move(current), std::move(selection),
        std::bind_front(
          &EditableTableView::make_table_item, this, std::move(view_builder)),
        std::move(comparator), parent) {
  get_header().get_item(0)->set_is_resizeable(false);
  get_header().get_widths()->set(0, scale_width(26));
  set_style(*this, TABLE_VIEW_STYLE());
  get_body().installEventFilter(this);
  m_current_connection = get_current()->connect_update_signal(
    std::bind_front(&EditableTableView::on_current, this));
}

bool EditableTableView::eventFilter(QObject* watched, QEvent* event) {
  if(watched == &get_body()) {
    if(event->type() == QEvent::KeyPress) {
      auto& key_event = *static_cast<QKeyEvent*>(event);
      if(key_event.key() == Qt::Key_Shift) {
        return true;
      }
    }
  }
  return TableView::eventFilter(watched, event);
}

bool EditableTableView::focusNextPrevChild(bool next) {
  setFocus();
  if(next) {
    navigate_next();
  } else {
    navigate_previous();
  }
  return true;
}

QWidget* EditableTableView::make_table_item(const ViewBuilder& view_builder,
    const std::shared_ptr<TableModel>& table, int row, int column) {
  if(column == 0) {
    auto button = make_delete_icon_button();
    button->setMaximumHeight(scale_height(26));
    match(*button, DeleteButton());
    auto tracker = std::make_shared<Tracker>(row);
    tracker->m_connection = get_table()->connect_operation_signal(
      std::bind_front(&TableRowIndexTracker::update, &tracker->m_index));
    button->connect_click_signal([=] {
      QTimer::singleShot(0, this, [=] {
        delete_row(tracker->m_index);
      });
    });
    return button;
  } else if(column == table->get_column_size() - 1) {
    return make_empty_cell();
  } else {
    auto item = view_builder(
      std::static_pointer_cast<EditableTableModel>(get_table())->m_source,
      any_cast<int>(table->at(row, 0)), column - 1);
    if(auto editable_box = find_editable_box(*item)) {
      editable_box->connect_end_edit_signal([=] {
        setFocus();
      });
    }
    return item;
  }
}

void EditableTableView::delete_row(const TableRowIndexTracker& row) {
  get_table()->remove(row.get_index());
}

void EditableTableView::navigate_next() {
  if(auto& current = get_current()->get()) {
    auto column = current->m_column + 1;
    if(column >= get_table()->get_column_size() - 1) {
      auto row = current->m_row + 1;
      if(row >= get_table()->get_row_size()) {
        get_current()->set(none);
      } else {
        get_current()->set(Index(row, 1));
      }
    } else {
      get_current()->set(Index(current->m_row, column));
    }
  } else {
    get_current()->set(Index(0, 1));
  }
}

void EditableTableView::navigate_previous() {
  if(auto& current = get_current()->get()) {
    auto column = current->m_column - 1;
    if(column <= 0) {
      auto row = current->m_row - 1;
      if(row < 0) {
        get_current()->set(none);
      } else {
        get_current()->set(Index(row, get_table()->get_column_size() - 1));
      }
    } else {
      get_current()->set(Index(current->m_row, column));
    }
  } else {
    get_current()->set(
      TableView::Index(get_table()->get_row_size() - 1,
        get_table()->get_column_size() - 1));
  }
}

void EditableTableView::on_current(const optional<Index>& index) {
  if(index) {
    if(index->m_column == 0) {
      navigate_next();
    } else if(index->m_column == get_table()->get_column_size() - 1) {
      navigate_previous();
    }
  }
}
