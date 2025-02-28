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
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/EmptyTableFilter.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/KeyInputBox.hpp"
#include "Spire/Ui/RecycledTableViewItemBuilder.hpp"
#include "Spire/Ui/SingleSelectionModel.hpp"
#include "Spire/Ui/StandardTableFilter.hpp"
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
    style.get(item_selector > Any() > ReadOnly() >
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
    optional<TableRowIndexTracker> m_index;
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
          m_transaction.push(AddOperation(operation.m_index + 1));
        },
        [&] (const MoveOperation& operation) {
          m_transaction.push(MoveOperation(operation.m_source + 1,
            operation.m_destination + 1));
        },
        [&] (const PreRemoveOperation& operation) {
          m_transaction.push(PreRemoveOperation(operation.m_index + 1));
        },
        [&] (const RemoveOperation& operation) {
          m_transaction.push(RemoveOperation(operation.m_index + 1));
        },
        [&] (const UpdateOperation& operation) {
          m_transaction.push(UpdateOperation(operation.m_index + 1,
            operation.get_previous(), operation.get_value()));
        },
        [&] (const auto& operation) {
          m_transaction.push(operation);
        });
    }
  };
}

struct EditableTableView::EditableItemBuilder {
  EditableTableView* m_view;
  std::unordered_map<QWidget*, std::shared_ptr<Tracker>> m_trackers;

  QWidget* mount(
      const std::shared_ptr<TableModel>& table, int row, int column) {
    if(column == 0) {
      auto button = make_delete_icon_button();
      button->setMaximumHeight(scale_height(26));
      match(*button, DeleteButton());
      auto tracker = std::make_shared<Tracker>(row);
      tracker->m_connection = table->connect_operation_signal(
        std::bind_front(&TableRowIndexTracker::update, &*tracker->m_index));
      m_trackers.insert(std::pair(button, tracker));
      button->connect_click_signal([=] {
        auto index = tracker->m_index->get_index();
        QTimer::singleShot(0, m_view, [=] {
          m_view->delete_row(index);
        });
      });
      return button;
    }
    return make_empty_cell();
  }

  void reset(QWidget& widget,
      const std::shared_ptr<TableModel>& table, int row, int column) {
    if(column != 0) {
      return;
    }
    auto tracker = m_trackers[&widget];
    tracker->m_index = none;
    tracker->m_index.emplace(row);
    tracker->m_connection = table->connect_operation_signal(
      std::bind_front(&TableRowIndexTracker::update, &*tracker->m_index));
  }

  void unmount(QWidget* widget) {
    delete widget;
  }
};

struct EditableTableView::ItemBuilder {
  EditableTableView* m_view;
  TableViewItemBuilder m_builder;
  RecycledTableViewItemBuilder<EditableItemBuilder> m_editable_builder;

  ItemBuilder(EditableTableView* view, TableViewItemBuilder builder)
    : m_view(view),
      m_builder(std::move(builder)),
      m_editable_builder(EditableItemBuilder(view)) {}

  QWidget* mount(
      const std::shared_ptr<TableModel>& table, int row, int column) {
    if(column == 0) {
      return m_editable_builder.mount(table, row, 0);
    } else if(column == table->get_column_size() - 1) {
      return m_editable_builder.mount(table, row, 1);
    } else {
      auto item = static_cast<EditableBox*>(m_builder.mount(
        std::static_pointer_cast<EditableTableModel>(
          m_view->get_table())->m_source, any_cast<int>(table->at(row, 0)),
          column - 1));
      item->connect_read_only_signal([=] (auto read_only) {
        if(read_only) {
          m_view->setFocus();
        }
      });
      return item;
    }
  }

  void unmount(QWidget* widget) {
    if(auto box = dynamic_cast<EditableBox*>(widget)) {
      m_builder.unmount(widget);
    } else {
      m_editable_builder.unmount(widget);
    }
  }
};

EditableTableView::EditableTableView(
    std::shared_ptr<TableModel> table, std::shared_ptr<HeaderModel> header,
    std::shared_ptr<TableFilter> table_filter,
    std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection,
    TableViewItemBuilder item_builder, Comparator comparator, QWidget* parent)
    : TableView(std::make_shared<EditableTableModel>(std::move(table), header),
        std::make_shared<EditableTableHeaderModel>(header),
        std::move(table_filter), std::make_shared<EditableTableCurrentModel>(
          std::move(current), header->get_size() + 2), std::move(selection),
        ItemBuilder(this, std::move(item_builder)), std::move(comparator),
        parent),
      m_is_processing_key(false) {
  get_header().get_item(0)->set_is_resizeable(false);
  get_header().get_widths()->set(0, scale_width(26));
  set_style(*this, TABLE_VIEW_STYLE());
}

void EditableTableView::keyPressEvent(QKeyEvent* event) {
  if(auto& current = get_body().get_current()->get()) {
    if(m_is_processing_key) {
      return TableView::keyPressEvent(event);
    }
    if(auto item = get_body().find_item(*current)) {
      m_is_processing_key = true;
      auto target = find_focus_proxy(item->get_body());
      QCoreApplication::sendEvent(target, event);
      target->setFocus();
      m_is_processing_key = false;
    }
  } else {
    TableView::keyPressEvent(event);
  }
}

void EditableTableView::delete_row(int row) {
  get_body().get_table()->remove(row);
}

EditableTableViewBuilder::EditableTableViewBuilder(
  std::shared_ptr<TableModel> table, QWidget* parent)
  : m_table(std::move(table)),
    m_parent(parent),
    m_header(std::make_shared<ArrayListModel<TableHeaderItem::Model>>()),
    m_filter(std::make_shared<EmptyTableFilter>()),
    m_current(std::make_shared<LocalValueModel<optional<TableIndex>>>()),
    m_selection(std::make_shared<TableSelectionModel>(
      std::make_shared<TableEmptySelectionModel>(),
      std::make_shared<ListSingleSelectionModel>(),
      std::make_shared<ListEmptySelectionModel>())),
    m_item_builder(&TableView::default_item_builder) {}

EditableTableViewBuilder& EditableTableViewBuilder::set_header(
    const std::shared_ptr<TableView::HeaderModel>& header) {
  m_header = header;
  return *this;
}

EditableTableViewBuilder&
    EditableTableViewBuilder::add_header_item(QString name) {
  return add_header_item(std::move(name), QString());
}

EditableTableViewBuilder& EditableTableViewBuilder::add_header_item(
    QString name, QString short_name) {
  return add_header_item(
    std::move(name), std::move(short_name), TableFilter::Filter::NONE);
}

EditableTableViewBuilder& EditableTableViewBuilder::add_header_item(
    QString name, TableHeaderItem::Order order) {
  return add_header_item(
    std::move(name), QString(), order, TableFilter::Filter::NONE);
}

EditableTableViewBuilder& EditableTableViewBuilder::add_header_item(
    QString name, TableFilter::Filter filter) {
  return add_header_item(std::move(name), QString(), filter);
}

EditableTableViewBuilder& EditableTableViewBuilder::add_header_item(
    QString name, QString short_name, TableFilter::Filter filter) {
  return add_header_item(std::move(name), std::move(short_name),
    TableHeaderItem::Order::NONE, filter);
}

EditableTableViewBuilder& EditableTableViewBuilder::add_header_item(
    QString name, QString short_name, TableHeaderItem::Order order,
    TableFilter::Filter filter) {
  m_header->push(TableHeaderItem::Model(std::move(name), std::move(short_name),
    order, filter));
  return *this;
}

EditableTableViewBuilder& EditableTableViewBuilder::set_filter(
    const std::shared_ptr<TableFilter>& filter) {
  m_filter = filter;
  return *this;
}

EditableTableViewBuilder& EditableTableViewBuilder::set_standard_filter() {
  if(m_table->get_row_size() == 0) {
    return *this;
  }
  auto types = std::vector<std::type_index>();
  for(auto i = 0; i != m_table->get_column_size(); ++i) {
    types.push_back(m_table->at(0, i).get_type());
  }
  return set_filter(std::make_shared<StandardTableFilter>(std::move(types)));
}

EditableTableViewBuilder& EditableTableViewBuilder::set_current(
    const std::shared_ptr<TableView::CurrentModel>& current) {
  m_current = current;
  return *this;
}

EditableTableViewBuilder& EditableTableViewBuilder::set_selection(
    const std::shared_ptr<TableView::SelectionModel>& selection) {
  m_selection = selection;
  return *this;
}

EditableTableViewBuilder& EditableTableViewBuilder::set_item_builder(
    const TableViewItemBuilder& item_builder) {
  m_item_builder = item_builder;
  return *this;
}

EditableTableViewBuilder& EditableTableViewBuilder::set_comparator(
    TableView::Comparator comparator) {
  m_comparator = comparator;
  return *this;
}

EditableTableView* EditableTableViewBuilder::make() const {
  return new EditableTableView(m_table, m_header, m_filter, m_current,
    m_selection, m_item_builder, m_comparator, m_parent);
}
