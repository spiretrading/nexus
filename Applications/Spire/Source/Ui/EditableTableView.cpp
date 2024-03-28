#include "Spire/Ui/EditableTableView.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TableItem.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using DeleteButton = StateSelector<void, struct DeleteButtonSeletorTag>;
  using EmptyCell = StateSelector<void, struct EmptyCellSeletorTag>;

  const auto DELETE_TIMEOUT_MS = 100;

  auto TABLE_VIEW_STYLE() {
    auto style = StyleSheet();
    style.get(Any() > is_a<ScrollBox>()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(PaddingBottom(64));
    style.get(Any() > Current()).
      set(BackgroundColor(Qt::transparent));
    style.get(Any() > HoverItem()).set(border_color(QColor(0xA0A0A0)));
    style.get((Any() > DeleteButton()) << is_a<TableItem>()).
      set(padding(scale_width(2)));
    style.get((Any() > DeleteButton()) << (HoverItem() || Current())).
      set(border_color(QColor(Qt::transparent)));
    style.get(Any() > (DeleteButton() && Hover()) > is_a<Icon>()).
      set(BackgroundColor(QColor(0xDFDFEB))).
      set(Fill(QColor(0xB71C1C)));
    style.get((Any() > EmptyCell()) << (HoverItem() || Current())).
      set(border_color(QColor(Qt::transparent)));
    style.get(Any() > CurrentRow() > DeleteButton()).
      set(Visibility(Visibility::VISIBLE));
    style.get(Any() > CurrentRow() > DeleteButton() > is_a<Icon>()).
      set(Fill(QColor(0x535353)));
    style.get(Any() > CurrentRow() > (DeleteButton() && Hover()) >
        is_a<Icon>()).
      set(BackgroundColor(QColor(0xD0CEEB))).
      set(Fill(QColor(0xB71C1C)));
    style.get(Any() > CurrentRow()).set(BackgroundColor(QColor(0xE2E0FF)));
    style.get(Any() > CurrentColumn()).set(BackgroundColor(Qt::transparent));
    return style;
  }

  auto DELETE_BUTTON_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).set(Visibility(Visibility::INVISIBLE));
    style.get(Any() > is_a<Box>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(horizontal_padding(scale_width(2))).
      set(vertical_padding(scale_height(2)));
    style.get(Any() > is_a<Icon>()).
      set(BackgroundColor(QColor(Qt::transparent)));
    return style;
  }

  auto TABLE_ROW_STYLE(StyleSheet style) {
    style.get(Any() > is_a<DropDownBox>() >
        (is_a<TextBox>() && !(+Any() << is_a<ListItem>()))).
      set(horizontal_padding(scale_width(8)));
    style.get(Any() > is_a<EditableBox>()).
      set(horizontal_padding(scale_width(8)));
    style.get(Hover()).set(BackgroundColor(0xF2F2FF));
    style.get(Hover() > DeleteButton()).
      set(Visibility(Visibility::VISIBLE));
    style.get(Hover() > DeleteButton() > is_a<Icon>()).
      set(Fill(QColor(0x535353)));
    return style;
  }

  QWidget* make_empty_cell() {
    auto cell = new QWidget();
    cell->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    match(*cell, EmptyCell());
    return cell;
  }
}

struct RevertColumnTableModel : TableModel {
  std::shared_ptr<TableModel> m_source;
  TableModelTransactionLog m_transaction;
  scoped_connection m_source_connection;

  explicit RevertColumnTableModel(std::shared_ptr<TableModel> source)
    : m_source(std::move(source)),
      m_source_connection(m_source->connect_operation_signal(
        std::bind_front(&RevertColumnTableModel::on_operation, this))) {}

  int get_row_size() const {
    return m_source->get_row_size();
  }

  int get_column_size() const {
    return m_source->get_column_size() - 2;
  }

  AnyRef at(int row, int column) const {
    return m_source->at(row, column + 1);
  }

  QValidator::State set(int row, int column, const std::any& value) {
    return m_source->set(row, column + 1, value);
  }

  connection connect_operation_signal(
      const OperationSignal::slot_type& slot) const {
    return m_transaction.connect_operation_signal(slot);
  }

  void on_operation(const TableModel::Operation& operation) {
    auto adjust_row = [] (int index, const AnyListModel& source) {
      auto row = std::make_shared<ArrayListModel<std::any>>();
      for(auto i = 1; i < source.get_size() - 1; ++i) {
        row->push(source.get(i));
      }
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
          operation.m_column - 1, operation.m_previous, operation.m_value));
      });
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

  int get_row_size() const {
    return m_source->get_row_size() + 1;
  }

  int get_column_size() const {
    return m_header->get_size() + 2;
  }

  AnyRef at(int row, int column) const {
    if(row < 0 || row >= get_row_size() || column < 0 ||
        column >= get_column_size()) {
      throw std::out_of_range("The row or column is out of range.");
    }
    if(column == 0) {
      static auto row_index = 0;
      row_index = row;
      return row_index;
    }
    column -= 1;
    if(row < m_source->get_row_size() && column < m_source->get_column_size()) {
      return m_source->at(row, column);
    }
    return {};
  }

  QValidator::State set(int row, int column, const std::any& value) {
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

  connection connect_operation_signal(
      const OperationSignal::slot_type& slot) const {
    return m_transaction.connect_operation_signal(slot);
  }

  void on_operation(const TableModel::Operation& operation) {
    auto adjust_row = [] (int index, const AnyListModel& source) {
      auto row = std::make_shared<ArrayListModel<std::any>>();
      static auto row_index = 0;
      row_index = index;
      row->push(row_index);
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
        std::bind_front(&EditableTableView::make_table_item, this,
          std::move(view_builder)),
        std::move(comparator), parent) {
  auto table_header = static_cast<TableHeader*>(static_cast<Box*>(
    layout()->itemAt(0)->widget())->get_body()->layout()->
      itemAt(0)->widget());
  auto empty_header_item =
    static_cast<TableHeaderItem*>(table_header->layout()->itemAt(0)->widget());
  empty_header_item->set_is_resizeable(false);
  table_header->get_widths()->set(0, scale_width(26));
  auto& scroll_box =
    *static_cast<ScrollBox*>(layout()->itemAt(1)->widget());
  scroll_box.setFocusPolicy(Qt::NoFocus);
  m_table_body = static_cast<TableBody*>(&scroll_box.get_body());
  m_table_body->installEventFilter(this);
  set_style(*this, TABLE_VIEW_STYLE());
  for(auto i = 0; i < get_table()->get_row_size(); ++i) {
    m_rows.push({});
  }
  for(auto i = 0; i < m_table_body->get_table()->get_row_size(); ++i) {
    auto row = m_table_body->layout()->itemAt(i)->widget();
    update_style(*row, [] (auto& style) {
      style = TABLE_ROW_STYLE(style);
    });
    m_rows.set(m_table_body->get_table()->get<int>(i, 0), row);
  }
  m_operation_connection = get_table()->connect_operation_signal(
    std::bind_front(&EditableTableView::on_source_table_operation, this));
}

connection EditableTableView::connect_delete_signal(
    const DeleteSignal::slot_type& slot) const {
  return m_delete_signal.connect(slot);
}

QWidget* EditableTableView::make_table_item(ViewBuilder source_view_builder,
    const std::shared_ptr<TableModel>& table, int row, int column) {
  if(column == 0) {
    if(row < table->get_row_size() - 1) {
      auto button = make_delete_icon_button();
      button->setMaximumHeight(scale_height(26));
      set_style(*button, DELETE_BUTTON_STYLE());
      match(*button, DeleteButton());
      button->connect_click_signal([=] {
        QTimer::singleShot(DELETE_TIMEOUT_MS,
          std::bind_front(&EditableTableView::delete_current_row, this));
      });
      return button;
    } else {
      return make_empty_cell();
    }
  } else if(column == table->get_column_size() - 1) {
    return make_empty_cell();
  } else {
    return source_view_builder(std::make_shared<RevertColumnTableModel>(table),
      row, column - 1);
  }
}

void EditableTableView::delete_current_row() {
  if(auto& current = get_current()->get()) {
    auto row = m_table_body->layout()->itemAt(current->m_row)->widget();
    auto source_index = [&] {
      for(auto i = 0; i < m_rows.get_size(); ++i) {
        if(m_rows.get(i) == row) {
          return i;
        }
      }
      return -1;
    }();
    if(source_index != -1) {
      m_delete_signal(source_index);
      m_table_body->setFocus();
      get_current()->set(TableBody::Index{get_current()->get()->m_row, 1});
    }
  }
}

void EditableTableView::on_source_table_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::RemoveOperation& operation) {
      m_rows.remove(operation.m_index);
      m_table_body->adjustSize();
      get_current()->set(get_current()->get());
    },
    [&] (const TableModel::MoveOperation& operation) {
      m_rows.move(operation.m_source, operation.m_destination);
    });
}
