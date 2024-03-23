#include "Spire/Ui/EditableTableView.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QApplication>
#include <QKeyEvent>
#include <QTimer>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/DropDownBox.hpp"
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
  using EmptyCell = StateSelector<void, struct EmptyCellSeletorTag>;
  using Editing = StateSelector<void, struct EditingSelectorTag>;
  using OutOfRangeRow = StateSelector<void, struct OutOfRangeRowSelectorTag>;

  const auto DELETE_TIMEOUT_MS = 100;

  auto TABLE_VIEW_STYLE() {
    auto style = StyleSheet();
    style.get(Any() > is_a<ScrollBox>()).
      set(BackgroundColor(QColor(0xFFFFFF))).
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
    style.get(Any() > OutOfRangeRow()).set(BackgroundColor(QColor(0xFDF8DE)));
    style.get(Any() > OutOfRangeRow() > (DeleteButton() && Hover()) >
        Body() > is_a<Icon>()).
      set(BackgroundColor(QColor(0xE9E4CC)));
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
    style.get(Hover() > Body() > is_a<Icon>()).
      set(BackgroundColor(QColor(0xDFDFEB))).
      set(Fill(QColor(0xB71C1C)));
    return style;
  }

  QWidget* make_empty_cell() {
    auto cell = new QWidget();
    cell->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    match(*cell, EmptyCell());
    return cell;
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

TransparentMouseEventPopupBox::TransparentMouseEventPopupBox(QWidget& body,
    QWidget* parent)
    : QWidget(parent),
      m_has_sent_event(false) {
  m_popup_box = new PopupBox(body);
  enclose(*this, *m_popup_box);
  proxy_style(*this, *m_popup_box);
  setFocusPolicy(Qt::ClickFocus);
  m_popup_box->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_tip_window = find_tip_window(body);
}

bool TransparentMouseEventPopupBox::eventFilter(
    QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(key_event.key() == Qt::Key_Tab) {
      focusNextChild();
      return true;
    } else if(key_event.key() == Qt::Key_Backtab) {
      focusPreviousChild();
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

bool TransparentMouseEventPopupBox::event(QEvent* event) {
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

void TransparentMouseEventPopupBox::showEvent(QShowEvent* event) {
  if(auto focus_proxy = find_focus_proxy(*m_popup_box)) {
    focus_proxy->installEventFilter(this);
  }
}

void TransparentMouseEventPopupBox::keyPressEvent(QKeyEvent* event) {
  if(m_has_sent_event) {
    return;
  }
  m_has_sent_event = true;
  QCoreApplication::sendEvent(&m_popup_box->get_body(), event);
  m_has_sent_event = false;
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

class EditableTableView::EditableTableRow {
  public:
    EditableTableRow(QWidget& row)
        : m_row(&row),
          m_is_ignore_filters(false),
          m_is_out_of_range(false) {
      update_style(*m_row, [] (auto& style) {
        style.get(Any() > is_a<DropDownBox>() >
            (is_a<TextBox>() && !(+Any() << is_a<ListItem>()))).
          set(horizontal_padding(scale_width(8)));
        style.get(Any() > is_a<EditableBox>()).
          set(horizontal_padding(scale_width(8)));
        style.get(Hover()).set(BackgroundColor(0xF2F2FF));
        style.get(Hover() > DeleteButton()).
          set(Visibility(Visibility::VISIBLE));
      });
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
      if(m_is_out_of_range == is_out_of_range) {
        return;
      }
      m_is_out_of_range = is_out_of_range;
      if(m_is_out_of_range) {
        match(*m_row, OutOfRangeRow());
      } else {
        unmatch(*m_row, OutOfRangeRow());
      }
    }

  private:
    QWidget* m_row;
    bool m_is_ignore_filters;
    bool m_is_out_of_range;
};

EditableTableView::EditableTableView(
    std::shared_ptr<TableModel> table, std::shared_ptr<HeaderModel> header,
    std::shared_ptr<TableFilter> table_filter,
    std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection, ViewBuilder view_builder,
    Comparator comparator, Filter filter, QWidget* parent)
    : TableView(std::make_shared<FilteredTableModel>(
        std::make_shared<EditableTableModel>(table, header), std::move(filter)),
      std::make_shared<EditableTableHeaderModel>(header),
      std::move(table_filter), std::move(current), std::move(selection),
      std::bind_front(&EditableTableView::view_builder, this, view_builder),
      std::move(comparator), parent),
      m_table(std::move(table)),
      m_newly_added_row(nullptr),
      m_has_sent_event(false),
      m_previous_table_row_size(m_table->get_row_size()) {
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
  for(auto i = 0; i < m_table->get_row_size() + 1; ++i) {
    m_source_rows.push({});
  }
  for(auto i = 0; i < m_table_body->get_table()->get_row_size(); ++i) {
    auto row = m_table_body->layout()->itemAt(i)->widget();
    m_rows[row] = std::make_unique<EditableTableRow>(*row);
    m_view_rows.push(row);
    m_source_rows.set(m_table_body->get_table()->get<int>(i, 0), row);
  }
  set_column_cover_mouse_events_transparent();
  m_source_operation_connection = m_table->connect_operation_signal(
    std::bind_front(&EditableTableView::on_source_table_operation, this));
  m_operation_connection = m_table_body->get_table()->connect_operation_signal(
    std::bind_front(&EditableTableView::on_table_operation, this));
  m_current_connection = get_current()->connect_update_signal(
    std::bind_front(&EditableTableView::on_current, this));
}

void EditableTableView::set_filter(const Filter& filter) {
  m_filter = filter;
  do_filter();
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
        case Qt::Key_Tab:
          navigate_next();
          return true;
        case Qt::Key_Backtab:
          navigate_previous();
          return true;
        case Qt::Key_Home:
        case Qt::Key_End:
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
          break;
        default:
          if(auto& current = get_current()->get()) {
            if(m_has_sent_event) {
              return true;
            }
            m_has_sent_event = true;
            QCoreApplication::sendEvent(find_focus_proxy(
              get_table_item_body(*m_table_body->get_item(*current))), event);
            m_has_sent_event = false;
            return true;
          }
      }
    }
  }
  return TableView::eventFilter(watched, event);
}

bool EditableTableView::focusNextPrevChild(bool next) {
  if(auto& current = get_current()->get(); !current) {
    auto focus_widget = QApplication::focusWidget();
    if(!m_table_body->isAncestorOf(focus_widget) &&
        m_table_body != focus_widget) {
      auto next_focus_widget = [&] {
        if(next) {
          return focus_widget->nextInFocusChain();
        }
        return focus_widget->previousInFocusChain();
      }();
      if(!m_table_body->isAncestorOf(next_focus_widget) &&
          m_table_body != focus_widget) {
        return QWidget::focusNextPrevChild(next);
      }
    }
  }
  m_table_body->setFocus();
  if(next) {
    navigate_next();
  } else {
    navigate_previous();
  }
  return true;
}

QWidget* EditableTableView::view_builder(ViewBuilder source_view_builder,
    const std::shared_ptr<TableModel>& table, int row, int column) {
  if(column == 0) {
    if(row < table->get_row_size() - 1) {
      auto button = make_delete_icon_button();
      button->setMaximumHeight(scale_height(26));
      set_style(*button, DELETE_BUTTON_STYLE());
      match(*button, DeleteButton());
      button->connect_click_signal([=] {
        QTimer::singleShot(DELETE_TIMEOUT_MS, [=] {
          delete_current_row();
        });
      });
      return button;
    } else {
      return make_empty_cell();
    }
  } else if(column == table->get_column_size() - 1) {
    return make_empty_cell();
  } else {
    auto cell = source_view_builder(
      std::make_shared<RevertTableModel>(table), row, column - 1);
    if(cell) {
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
    }
    return cell;
  }
}

void EditableTableView::set_column_cover_mouse_events_transparent() {
  auto& children = m_table_body->children();
  auto count = 0;
  for(auto i = children.rbegin(); i != children.rend(); ++i) {
    if(count >= get_table()->get_column_size()) {
      break;
    }
    if((*i)->isWidgetType() &&
        m_table_body->layout()->indexOf(static_cast<QWidget*>(*i)) == -1) {
      static_cast<QWidget*>(*i)->setAttribute(Qt::WA_TransparentForMouseEvents);
      ++count;
    }
  }
}

void EditableTableView::delete_current_row() {
  if(auto& current = get_current()->get()) {
    auto source_index = [&] {
      for(auto i = 0; i < m_source_rows.get_size(); ++i) {
        if(m_source_rows.get(i) == m_view_rows.get(current->m_row)) {
          return i;
        }
      }
      return -1;
    }();
    if(source_index >= 0) {
      m_delete_signal(source_index);
    }
  }
  m_table_body->setFocus();
  if(get_current()->get()) {
    get_current()->set(TableBody::Index{get_current()->get()->m_row, 1});
  }
}

void EditableTableView::do_filter() {
  std::static_pointer_cast<FilteredTableModel>(get_table())->set_filter(
    [=] (const TableModel& model, int row) {
      if(row == model.get_row_size() - 1) {
        return false;
      }
      return m_filter(model, row);
    });
}

void EditableTableView::navigate_next() {
  if(auto& current = get_current()->get()) {
    auto column = current->m_column + 1;
    if(column >= m_table_body->get_table()->get_column_size() - 1) {
      auto row = current->m_row + 1;
      if(row >= m_table_body->get_table()->get_row_size()) {
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
        QWidget::focusNextPrevChild(false);
        get_current()->set(none);
      } else {
        get_current()->set(Index(row,
          m_table_body->get_table()->get_column_size() - 1));
      }
    } else {
      get_current()->set(Index(current->m_row, column));
    }
  } else {
    get_current()->set(
      TableView::Index(m_table_body->get_table()->get_row_size() - 1,
        m_table_body->get_table()->get_column_size() - 1));
  }
}

void EditableTableView::on_current(const optional<Index>& index) {
  if(index) {
    if(index->m_column == 0) {
      navigate_next();
    } else if(index->m_column == get_table()->get_column_size() - 1 &&
        index->m_column - 1 > 0) {
      navigate_previous();
    }
  }
}

void EditableTableView::on_table_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      auto row_widget =
        m_table_body->layout()->itemAt(operation.m_index)->widget();
      m_view_rows.insert(row_widget, operation.m_index);
      m_rows[row_widget] = std::make_unique<EditableTableRow>(*row_widget);
      m_source_rows.set(
        m_table_body->get_table()->get<int>(operation.m_index, 0), row_widget);
      if(m_previous_table_row_size < m_table->get_row_size()) {
        m_newly_added_row = row_widget;
        if(auto& current = get_current()->get()) {
          auto index = TableView::Index(operation.m_index, current->m_column);
          get_current()->set(index);
        }
      }
    },
    [&] (const TableModel::RemoveOperation& operation) {
      m_view_rows.remove(operation.m_index);
    },
    [&] (const TableModel::MoveOperation& operation) {
      m_view_rows.move(operation.m_source, operation.m_destination);
    });
}

void EditableTableView::on_source_table_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      m_previous_table_row_size = m_table->get_row_size();
      m_newly_added_row->show();
      m_source_rows.insert(m_newly_added_row, operation.m_index);
      m_table_body->adjustSize();
      get_current()->set(get_current()->get());
    },
    [&] (const TableModel::RemoveOperation& operation) {
      m_previous_table_row_size = m_table->get_row_size();
      m_source_rows.remove(operation.m_index);
      m_table_body->adjustSize();
      get_current()->set(get_current()->get());
    },
    [&] (const TableModel::MoveOperation& operation) {
      m_source_rows.move(operation.m_source, operation.m_destination);
    });
}
