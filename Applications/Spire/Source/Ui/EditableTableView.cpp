#include "Spire/Ui/EditableTableView.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QApplication>
#include <QLayout>
#include <QKeyEvent>
#include <QTimer>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/PopupBox.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TableItem.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using DeleteButton = StateSelector<void, struct DeleteButtonSeletorTag>;

  using HoverRow = StateSelector<void, struct HoverRowSelectorTag>;

  using EmptyCell = StateSelector<void, struct EmptyCellSeletorTag>;

  using Editing = StateSelector<void, struct EditingSelectorTag>;

  const auto DELETE_TIMEOUT_MS = 200;

  bool is_a_word(const QString& text) {
    return text.size() == 1 && (text[0].isLetterOrNumber() || text[0] == '_');
  }

  auto is_in_layout(QLayout* layout, QWidget* widget) {
    for(auto i = 0; i < layout->count(); ++i) {
      if(layout->itemAt(i)->widget() == widget) {
        return true;
      }
    }
    return false;
  }

  QWidget& get_table_item_body(const TableItem& item) {
    return *item.layout()->itemAt(0)->widget();
  }

  QWidget* find_tip_window(const QWidget& parent) {
    for(auto child : parent.children()) {
      if(!child->isWidgetType()) {
        continue;
      }
      auto& widget = *static_cast<QWidget*>(child);
      if(widget.isWindow() &&
          (widget.windowFlags() & Qt::WindowDoesNotAcceptFocus)) {
        return &widget;
      }
      if(auto window = find_tip_window(widget)) {
        return window;
      }
    }
    return nullptr;
  }

  EditableBox* find_editable_box(const QWidget& widget) {
    if(auto editable_box =
        dynamic_cast<EditableBox*>(const_cast<QWidget*>(&widget))) {
      return editable_box;
    }
    if(auto layout = widget.layout(); layout && layout->count() == 1) {
      return find_editable_box(*layout->itemAt(0)->widget());
    }
    return nullptr;
  }
}

CustomPopupBox::CustomPopupBox(QWidget& body, QWidget* parent)
    : QWidget(parent) {
  m_popup_box = new PopupBox(body);
  enclose(*this, *m_popup_box);
  proxy_style(*this, *m_popup_box);
  setFocusProxy(m_popup_box);
  setFocusPolicy(Qt::ClickFocus);
  m_popup_box->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_tip_window = find_tip_window(body);
}

bool CustomPopupBox::event(QEvent* event) {
  switch(event->type()) {
    case QEvent::MouseButtonPress:
      if(auto& mouse_event = *static_cast<QMouseEvent*>(event);
        mouse_event.button() == Qt::LeftButton) {
        m_popup_box->get_body().setFocus();
      }
      break;
    case QEvent::Enter:
    case QEvent::Leave:
      if(m_tip_window) {
        QCoreApplication::sendEvent(m_tip_window->parentWidget(), event);
      }
      break;
  }
  return QWidget::event(event);
}

struct RevertTableModel : TableModel {
  std::shared_ptr<TableModel> m_source;
  TableModelTransactionLog m_transaction;
  scoped_connection m_source_connection;

  explicit RevertTableModel(std::shared_ptr<TableModel> source)
    : m_source(std::move(source)),
      m_source_connection(m_source->connect_operation_signal(
        std::bind_front(&RevertTableModel::on_operation, this))) {}

  int get_row_size() const {
    return m_source->get_row_size();
  }

  int get_column_size() const {
    return m_source->get_column_size() - 1;
  }

  AnyRef at(int row, int column) const {
    if(row < 0 || row >= get_row_size() || column < 0 ||
        column >= get_column_size()) {
      throw std::out_of_range("The row or column is out of range.");
    }
    return m_source->at(row, column + 1);
  }

  QValidator::State set(int row, int column, const std::any& value) {
    if(row < 0 || row >= get_row_size() || column < 0 ||
        column >= get_column_size()) {
      throw std::out_of_range("The row or column is out of range.");
    }
    return m_source->set(row, column + 1, value);
  }

  connection connect_operation_signal(
      const OperationSignal::slot_type& slot) const {
    return m_transaction.connect_operation_signal(slot);
  }

  void on_operation(const TableModel::Operation& operation) {
    auto adjust_row = [] (int index, const AnyListModel & source) {
      auto row = std::make_shared<ArrayListModel<std::any>>();
      for(auto i = 1; i < source.get_size(); ++i) {
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

struct EditableTableView::EditableTableModel : TableModel {
  std::shared_ptr<TableModel> m_source;
  TableModelTransactionLog m_transaction;
  scoped_connection m_source_connection;

  explicit EditableTableModel(std::shared_ptr<TableModel> source)
    : m_source(std::move(source)),
      m_source_connection(m_source->connect_operation_signal(
        std::bind_front(&EditableTableModel::on_operation, this))) {}

  int get_row_size() const {
    return m_source->get_row_size() + 1;
  }

  int get_column_size() const {
    return m_source->get_column_size() + 1;
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
    if(row < m_source->get_row_size()) {
      return m_source->at(row, column - 1);
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
    if(row < m_source->get_row_size()) {
      return m_source->set(row, column - 1, value);
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

struct EditableTableView::EditableTableHeaderModel :
    ListModel<TableHeaderItem::Model> {
  std::shared_ptr<ListModel<TableHeaderItem::Model>> m_source;
  ListModelTransactionLog<TableHeaderItem::Model> m_transaction;
  scoped_connection m_source_connection;

  explicit EditableTableHeaderModel(
    std::shared_ptr<ListModel<TableHeaderItem::Model>> source)
    : m_source(std::move(source)),
      m_source_connection(m_source->connect_operation_signal(
        std::bind_front(&EditableTableHeaderModel::on_operation, this))) {}

  int get_size() const override {
    return m_source->get_size() + 1;
  }

  const TableHeaderItem::Model& get(int index) const override {
    if(index == 0) {
      static auto model = TableHeaderItem::Model{"", "",
        TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE};
      return model;
    }
    return m_source->get(index - 1);
  }

  QValidator::State set(int index,
      const TableHeaderItem::Model& value) override {
    if(index == 0) {
      return QValidator::Invalid;
    }
    return m_source->set(index - 1, value);
  }

  QValidator::State insert(const TableHeaderItem::Model& value,
      int index) override {
    if(index == 0) {
      return QValidator::Invalid;
    }
    return m_source->insert(value, index - 1);
  }

  QValidator::State move(int source, int destination) override {
    if(source == 0 || destination == 0) {
      return QValidator::Invalid;
    }
    return m_source->move(source - 1, destination - 1);
  }

  QValidator::State remove(int index) override {
    if(index == 0) {
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
    m_transaction.push(Operation(operation));
  }
};

class EditableTableView::EditableTableRow {
  public:
    EditableTableRow(std::shared_ptr<TableModel> source, int row_index,
        QWidget& row)
        : m_source(std::move(source)),
          m_row_index(row_index),
          m_row(&row),
          m_is_ignore_filters(false),
          m_is_out_of_range(false),
          m_operation_connection(m_source->connect_operation_signal(
            std::bind_front(&EditableTableRow::on_operation, this))) {
      update_style(*m_row, [] (auto& style) {
        style.get(Hover()).
          set(BackgroundColor(0xF2F2FF));
        style.get(Hover() > DeleteButton()).
          set(Visibility(Visibility::VISIBLE));
      });
    }

    void set_row_index(int index) {
      m_row_index = index;
    }

    int get_row_index() const {
      return m_row_index;
    }

    QWidget* get_row() const {
      return m_row;
    }

    bool is_ignore_filters() const {
      return m_is_ignore_filters;
    }

    void set_ignore_filters(bool is_ignore_filters) {
      m_is_ignore_filters = is_ignore_filters;
    }

    bool is_out_of_range() const {
      return m_is_out_of_range;
    }

    void set_out_of_range(bool is_out_of_range) {
      m_is_out_of_range = is_out_of_range;
    }

  private:
    std::shared_ptr<TableModel> m_source;
    int m_row_index;
    QWidget* m_row;
    bool m_is_ignore_filters;
    bool m_is_out_of_range;
    scoped_connection m_operation_connection;

    void on_operation(const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::AddOperation& operation) {
          if(m_row_index >= operation.m_index) {
            ++m_row_index;
          }
        },
        [&] (const TableModel::RemoveOperation& operation) {
          if(m_row_index == operation.m_index) {
            m_row_index = -1;
          } else if(m_row_index > operation.m_index) {
            --m_row_index;
          }
        },
        [&] (const TableModel::MoveOperation& operation) {
          if(m_row_index == operation.m_source) {
            m_row_index = operation.m_destination;
          } else if(operation.m_source < operation.m_destination) {
            if(m_row_index > operation.m_source &&
              m_row_index <= operation.m_destination) {
              --m_row_index;
            }
          } else if(m_row_index >= operation.m_destination &&
            m_row_index < operation.m_source) {
            ++m_row_index;
          }
        });
    }
};

EditableTableView::EditableTableView(
    std::shared_ptr<TableModel> table, std::shared_ptr<HeaderModel> header,
    std::shared_ptr<TableFilter> table_filter, std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection, ViewBuilder view_builder,
    Comparator comparator, Filter filter, QWidget* parent)
    : TableView(std::make_shared<EditableTableModel>(
        std::make_shared<FilteredTableModel>(table, std::move(filter))),
      std::make_shared<EditableTableHeaderModel>(std::move(header)),
      std::move(table_filter), std::move(current), std::move(selection),
      std::bind_front(&EditableTableView::view_builder, this, view_builder),
      std::move(comparator), parent),
      m_source_table(std::move(table)) {
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
  update_style(*this, [] (auto& style) {
    style.get(Any() > is_a<ScrollBox>()).
      set(PaddingBottom(64));
    style.get(Any() > Current()).
      set(BackgroundColor(Qt::transparent));
    style.get((Any() > Editing()) << Current()).
      set(BackgroundColor(Qt::transparent)).
      set(border_color(QColor(Qt::transparent)));
    style.get(Any() > HoverItem()).set(border_color(QColor(0xA0A0A0)));
    style.get((Any() > DeleteButton()) << is_a<TableItem>()).
      set(padding(scale_width(2)));
    style.get((Any() > DeleteButton()) << (HoverItem() || Current())).
      set(border_color(QColor(Qt::transparent)));
    style.get((Any() > EmptyCell()) << (HoverItem() || Current())).
      set(border_color(QColor(Qt::transparent)));
    style.get(Any() > CurrentRow() > DeleteButton()).
      set(Visibility(Visibility::VISIBLE));
    style.get(Any() > CurrentRow() > (DeleteButton() && Hover()) >
        Body() > is_a<Icon>()).
      set(BackgroundColor(QColor(0xD0CEEB)));
    style.get(Any() > CurrentRow()).set(BackgroundColor(QColor(0xE2E0FF)));
    style.get(Any() > CurrentColumn()).set(BackgroundColor(Qt::transparent));
  });
  for(auto row = 0; row < get_table()->get_row_size(); ++row) {
    auto editable_row = std::make_shared<EditableTableRow>(m_source_table, row,
      *m_table_body->layout()->itemAt(row)->widget());
    if(row == get_table()->get_row_size() - 1) {
      editable_row->set_ignore_filters(true);
    }
    editable_row->get_row()->raise();
    m_rows.push(editable_row);
  }
  auto& children = m_table_body->children();
  auto count = 0;
  for(auto i = children.rbegin(); i != children.rend(); ++i) {
    if(count >= get_table()->get_column_size()) {
      break;
    }
    if((*i)->isWidgetType() &&
        !is_in_layout(m_table_body->layout(), static_cast<QWidget*>(*i))) {
      static_cast<QWidget*>(*i)->hide();
      ++count;
    }
  }
  m_source_operation_connection = m_source_table->connect_operation_signal(
    std::bind_front(&EditableTableView::on_source_table_operation, this));
  m_operation_connection = m_table_body->get_table()->connect_operation_signal(
    std::bind_front(&EditableTableView::on_table_operation, this));
  m_current_connection = get_current()->connect_update_signal(
    std::bind_front(&EditableTableView::on_current, this));
  m_sort_connection = connect_sort_signal(
    std::bind_front(&EditableTableView::on_sort, this));
}

void EditableTableView::set_filter(const Filter& filter) {
  auto current = get_current()->get();
  std::static_pointer_cast<FilteredTableModel>(
    std::static_pointer_cast<EditableTableModel>(
      get_table())->m_source)->set_filter(
        [=] (const TableModel& model, int row) {
          return filter(model, row);
        });
}

connection EditableTableView::connect_delete_signal(
    const DeleteSignal::slot_type& slot) const {
  return m_delete_signal.connect(slot);
}

bool EditableTableView::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_table_body) {
    if(event->type() == QEvent::KeyPress) {
      auto& key_event = *static_cast<QKeyEvent*>(event);
      switch(key_event.key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Backspace:
          if(auto current = m_table_body->get_current()->get();
              current->m_column != 0) {
            QCoreApplication::sendEvent(find_focus_proxy(
              get_table_item_body(*m_table_body->get_item(*current))), event);
            return true;
          }
          break;
        case Qt::Key_Shift:
          return true;
        case Qt::Key_A:
          if(key_event.modifiers() & Qt::Modifier::CTRL &&
              !key_event.isAutoRepeat()) {
            return true;
          }
        default:
          if(auto text = key_event.text(); is_a_word(text)) {
            if(auto current = m_table_body->get_current()->get();
                current->m_column != 0) {
              QCoreApplication::sendEvent(find_focus_proxy(
                get_table_item_body(*m_table_body->get_item(*current))), event);
              return true;
            }
          }
      }
    }
  }
  return TableView::eventFilter(watched, event);
}

QWidget* EditableTableView::view_builder(ViewBuilder source_view_builder,
    const std::shared_ptr<TableModel>& table, int row, int column) {
  if(column == 0) {
    if(row < table->get_row_size() - 1) {
      auto button = make_delete_icon_button();
      match(*button, DeleteButton());
      update_style(*button, [&] (auto& style) {
        style.get(Any()).
          set(Visibility(Visibility::INVISIBLE));
        style.get(Hover() > Body() > is_a<Icon>()).
          set(BackgroundColor(QColor(0xDFDFEB))).
          set(Fill(QColor(0xB71C1C)));
      });
      button->connect_click_signal([=] {
        QTimer::singleShot(DELETE_TIMEOUT_MS, [=] {
          m_delete_signal(m_rows.get(
            m_table_body->get_current()->get()->m_row)->get_row_index());
          m_table_body->setFocus();
        });
      });
      return button;
    } else {
      auto label = make_label("");
      label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      match(*label, EmptyCell());
      return label;
    }
  } else {
    auto cell = source_view_builder(
      std::make_shared<RevertTableModel>(table), row, column - 1);
    cell->setFocusPolicy(Qt::ClickFocus);
    if(auto editable_box = find_editable_box(*cell)) {
      editable_box->connect_start_edit_signal([=] {
        match(*cell, Editing());
      });
      editable_box->connect_end_edit_signal([=] {
        unmatch(*cell, Editing());
        if(!QApplication::focusWidget()) {
          m_table_body->setFocus();
        }
      });
    }
    return cell;
  }
}

void EditableTableView::on_current(const boost::optional<Index>& index) {
  if(index) {
    if(index->m_column == 0) {
      m_table_body->get_current()->set(TableBody::Index{index->m_row, 1});
      return;
    }
  }
}

void EditableTableView::on_table_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      if(auto current = m_table_body->get_current()->get()) {
        auto index = TableView::Index(operation.m_index, current->m_column);
        auto blocker = shared_connection_block(m_current_connection);
        m_table_body->get_current()->set(index);
      }
      auto row = m_table_body->layout()->itemAt(operation.m_index)->widget();
      auto editable_row = std::make_shared<EditableTableRow>(m_source_table,
        m_table_body->get_table()->get<int>(operation.m_index, 0), *row);
      m_rows.insert(editable_row, operation.m_index);
      row->show();
    },
    [&] (const TableModel::RemoveOperation& operation) {
      m_rows.remove(operation.m_index);
    },
    [&] (const TableModel::MoveOperation& operation) {
      m_rows.move(operation.m_source, operation.m_destination);
    });
}

void EditableTableView::on_source_table_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
    },
    [&] (const TableModel::RemoveOperation& operation) {
    },
    [&] (const TableModel::MoveOperation& operation) {
    });
}

void EditableTableView::on_sort(int column, TableHeaderItem::Order order) {
  m_table_body->update();
}
