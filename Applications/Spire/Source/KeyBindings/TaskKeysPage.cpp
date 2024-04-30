#include "Spire/KeyBindings/TaskKeysPage.hpp"
#include "Spire/KeyBindings/OrderTaskArgumentsMatch.hpp"
#include "Spire/KeyBindings/TaskKeysTableView.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/LineInputForm.hpp"
#include "Spire/Ui/SearchBox.hpp"
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextAreaBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto make_help_text_box() {
    auto help_text_box = make_text_area_label(
      QObject::tr("Allowed keys are: <b>F1–F12</b> and <b>Ctrl, Shift, "
        "Alt  +  F1–F12</b> and <b>Ctrl, Shift, Alt  +  0–9</b>"));
    help_text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    update_style(*help_text_box, [] (auto& style) {
      auto font = QFont("Roboto");
      font.setWeight(QFont::Normal);
      font.setPixelSize(scale_width(10));
      style.get(Any()).
        set(text_style(font, QColor(0x808080))).
        set(horizontal_padding(scale_width(8))).
        set(PaddingTop(scale_height(10))).
        set(PaddingBottom(scale_height(5)));
    });
    return help_text_box;
  }

  auto make_button(const QString& path, const QString& tooltip) {
    auto button = make_icon_button(imageFromSvg(path, scale(16, 16)), tooltip);
    button->setFixedSize(scale(26, 26));
    button->setFocusPolicy(Qt::TabFocus);
    return button;
  }
}

struct OrderTaskMatchCache {
  std::unordered_set<QString> m_caches;

  bool matches(const OrderTaskArguments& order_task, const QString& query) {
    if(m_caches.contains(query)) {
      return true;
    }
    auto matched = ::matches(order_task, query);
    if(matched) {
      m_caches.insert(query);
    }
    return matched;
  }
};

struct TaskKeysPage::FilteredTaskKeysListModel : OrderTaskArgumentsListModel {
  std::shared_ptr<OrderTaskArgumentsListModel> m_source;
  std::vector<OrderTaskMatchCache> m_caches;
  std::vector<int> m_filtered_data;
  QString m_query;
  ListModelTransactionLog<OrderTaskArguments> m_transaction;
  scoped_connection m_source_connection;

  explicit FilteredTaskKeysListModel(
      std::shared_ptr<OrderTaskArgumentsListModel> source)
      : m_source(std::move(source)),
        m_filtered_data(m_source->get_size()),
        m_source_connection(m_source->connect_operation_signal(
          std::bind_front(&FilteredTaskKeysListModel::on_operation, this))) {
    std::iota(m_filtered_data.begin(), m_filtered_data.end(), 0);
  }

  void matches(const QString& query) {
    m_query = query;
    auto source_row = 0;
    auto filtered_row = 0;
    m_transaction.transact([&] {
      while(source_row != m_source->get_size() &&
          filtered_row != static_cast<int>(m_filtered_data.size())) {
        if(is_matched(source_row)) {
          if(m_filtered_data[filtered_row] != source_row) {
            m_filtered_data.insert(
              m_filtered_data.begin() + filtered_row, source_row);
            m_transaction.push(AddOperation(filtered_row, get(filtered_row)));
          }
          ++filtered_row;
        } else {
          if(m_filtered_data[filtered_row] == source_row) {
            auto& order_task = get(filtered_row);
            m_filtered_data.erase(m_filtered_data.begin() + filtered_row);
            m_transaction.push(
              RemoveOperation(filtered_row, std::move(order_task)));
          }
        }
        ++source_row;
      }
      while(filtered_row != static_cast<int>(m_filtered_data.size())) {
        auto& order_task = get(filtered_row);
        m_filtered_data.erase(m_filtered_data.begin() + filtered_row);
        m_transaction.push(
          RemoveOperation(filtered_row, std::move(order_task)));
      }
      while(source_row != m_source->get_size()) {
        if(is_matched(source_row)) {
          m_filtered_data.push_back(source_row);
          m_transaction.push(AddOperation(
            m_filtered_data.size() - 1, get(m_filtered_data.size() - 1)));
        }
        ++source_row;
      }
    });
  }

  int get_size() const override {
    return static_cast<int>(m_filtered_data.size());
  }

  const OrderTaskArguments& get(int index) const override {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    return m_source->get(m_filtered_data[index]);
  }

  QValidator::State set(int index, const OrderTaskArguments& value) override {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    return m_source->set(m_filtered_data[index], value);
  }

  QValidator::State insert(const OrderTaskArguments& value,
      int index) override {
    if(index < 0 || index > get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    auto source_index = [&] {
      if(index < get_size()) {
        return m_filtered_data[index];
      }
      return m_source->get_size();
    }();
    return m_source->insert(value, source_index);
  }

  QValidator::State remove(int index) override {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    return m_source->remove(m_filtered_data[index]);
  }

  connection connect_operation_signal(
      const OperationSignal::slot_type& slot) const override {
    return m_transaction.connect_operation_signal(slot);
  }

  void transact(const std::function<void()>& transaction) override {
    m_transaction.transact(transaction);
  }

  bool is_matched(int index) {
    return m_query.isEmpty() ||
      m_caches[index].matches(m_source->get(index), m_query);
  }

  std::tuple<bool, std::vector<int>::iterator> find(int index) {
    auto i = std::lower_bound(m_filtered_data.begin(), m_filtered_data.end(),
      index);
    if(i != m_filtered_data.end() && *i == index) {
      return {true, i};
    }
    return {false, i};
  }

  void on_operation(const Operation& operation) {
    visit(operation,
      [&] (const StartTransaction&) {
        m_transaction.start();
      },
      [&] (const EndTransaction&) {
        m_transaction.end();
      },
      [&] (const OrderTaskArgumentsListModel::AddOperation& operation) {
        m_caches.insert(std::next(m_caches.begin(), operation.m_index),
          OrderTaskMatchCache());
        if(operation.m_index >= m_source->get_size() - 1) {
          if(is_matched(operation.m_index)) {
            m_filtered_data.push_back(operation.m_index);
            m_transaction.push(AddOperation(
              static_cast<int>(m_filtered_data.size()) - 1,
              std::any_cast<const OrderTaskArguments&>(operation.m_value)));
          }
        } else {
          auto i = std::get<1>(find(operation.m_index));
          std::for_each(i, m_filtered_data.end(), [] (int& value) { ++value; });
          if(is_matched(operation.m_index)) {
            m_transaction.push(AddOperation(
              static_cast<int>(m_filtered_data.insert(i, operation.m_index) -
                m_filtered_data.begin()),
              std::any_cast<const OrderTaskArguments&>(operation.m_value)));
          }
        }
      },
      [&] (const OrderTaskArgumentsListModel::RemoveOperation& operation) {
        m_caches.erase(std::next(m_caches.begin(), operation.m_index));
        auto [is_found, i] = find(operation.m_index);
        std::for_each(i, m_filtered_data.end(), [] (int& value) { --value; });
        if(is_found) {
          auto index = static_cast<int>(i - m_filtered_data.begin());
          m_filtered_data.erase(i);
          m_transaction.push(RemoveOperation(index,
            std::any_cast<const OrderTaskArguments&>(operation.m_value)));
        }
      },
      [&] (const OrderTaskArgumentsListModel::UpdateOperation& operation) {
        m_caches[operation.m_index] = OrderTaskMatchCache();
        auto [is_found, i] = find(operation.m_index);
        if(is_matched(operation.m_index)) {
          if(is_found) {
            m_transaction.push(UpdateOperation(
              static_cast<int>(i - m_filtered_data.begin()),
              std::any_cast<const OrderTaskArguments&>(operation.m_previous),
              std::any_cast<const OrderTaskArguments&>(operation.m_value)));
          } else {
            auto filtered_row = static_cast<int>(m_filtered_data.insert(
              i, operation.m_index) - m_filtered_data.begin());
            m_transaction.push(AddOperation(filtered_row, get(filtered_row)));
          }
        } else if(is_found) {
          auto index = static_cast<int>(i - m_filtered_data.begin());
          auto& order_task = get(index);
          m_filtered_data.erase(i);
          m_transaction.push(RemoveOperation(index, std::move(order_task)));
        }
      });
  }
};

TaskKeysPage::TaskKeysPage(std::shared_ptr<KeyBindingsModel> key_bindings,
    DestinationDatabase destinations, MarketDatabase markets, QWidget* parent)
    : QWidget(parent),
      m_key_bindings(std::move(key_bindings)),
      m_filtered_model(std::make_shared<FilteredTaskKeysListModel>(
        m_key_bindings->get_order_task_arguments())),
      m_is_row_added(false) {
  auto toolbar_body = new QWidget();
  auto toolbar_layout = make_hbox_layout(toolbar_body);
  auto search_box = new SearchBox();
  search_box->set_placeholder(tr("Search tasks"));
  search_box->setFixedWidth(scale_width(368));
  search_box->get_current()->connect_update_signal(
    std::bind_front(&TaskKeysPage::on_search, this));
  toolbar_layout->addWidget(search_box);
  toolbar_layout->addStretch();
  toolbar_layout->addSpacing(scale_width(18));
  auto add_task_button =
    make_button(":/Icons/add.svg", tr("Add Task (Shift + Enter)"));
  add_task_button->connect_click_signal(
    std::bind_front(&TaskKeysPage::on_new_task_action, this));
  toolbar_layout->addWidget(add_task_button);
  toolbar_layout->addSpacing(scale_width(4));
  m_duplicate_button =
    make_button(":/Icons/duplicate.svg", tr("Duplicate (Ctrl + D)"));
  m_duplicate_button->connect_click_signal(
    std::bind_front(&TaskKeysPage::on_duplicate_task_action, this));
  toolbar_layout->addWidget(m_duplicate_button);
  toolbar_layout->addSpacing(scale_width(4));
  m_delete_button = make_button(":/Icons/delete3.svg", tr("Delete"));
  m_delete_button->connect_click_signal(
    std::bind_front(&TaskKeysPage::on_delete_task_action, this));
  update_style(*m_delete_button, [] (auto& style) {
    style.get((Hover() || Press()) > Body() > is_a<Icon>()).
      set(Fill(QColor(0xB71C1C)));
  });
  toolbar_layout->addWidget(m_delete_button);
  auto toolbar = new Box(toolbar_body);
  update_style(*toolbar, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(horizontal_padding(scale_width(8))).
      set(PaddingTop(scale_height(5))).
      set(PaddingBottom(scale_height(10))).
      set(BorderBottomSize(scale_height(1))).
      set(BorderBottomColor(QColor(0xE0E0E0)));
  });
  auto body = new QWidget();
  auto layout = make_vbox_layout(body);
  layout->addWidget(make_help_text_box());
  layout->addWidget(toolbar);
  m_table_view = make_task_keys_table_view(
    m_filtered_model, std::make_shared<LocalComboBoxQueryModel>(),
    std::move(destinations), std::move(markets));
  layout->addWidget(m_table_view);
  auto box = new Box(body);
  update_style(*box, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
  });
  enclose(*this, *box);
  update_button_state();
  auto& row_selection = m_table_view->get_selection()->get_row_selection();
  m_selection_connection = row_selection->connect_operation_signal(
    std::bind_front(&TaskKeysPage::on_row_selection, this));
  m_table_operation_connection =
    m_table_view->get_body().get_table()->connect_operation_signal(
      std::bind_front(&TaskKeysPage::on_table_operation, this));
}

const std::shared_ptr<KeyBindingsModel>&
    TaskKeysPage::get_key_bindings() const {
  return m_key_bindings;
}

bool TaskKeysPage::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_added_region_item) {
    if(event->type() == QEvent::Show) {
      m_added_region_item->removeEventFilter(this);
      static_cast<EditableBox*>(
        &m_added_region_item->get_body())->set_editing(true);
    }
  }
  return QWidget::eventFilter(watched, event);
}

void TaskKeysPage::keyPressEvent(QKeyEvent* event) {
  if(event->modifiers() & Qt::ShiftModifier &&
      (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
    on_new_task_action();
  } else if(event->modifiers() & Qt::ControlModifier &&
      event->key() == Qt::Key_D) {
    on_duplicate_task_action();
  } else {
    QWidget::keyPressEvent(event);
  }
}

void TaskKeysPage::update_button_state() {
  auto is_enabled =
    m_table_view->get_selection()->get_row_selection()->get_size() > 0;
  m_duplicate_button->setEnabled(is_enabled);
  m_delete_button->setEnabled(is_enabled);
}

void TaskKeysPage::on_search(const QString& query) {
  m_filtered_model->matches(query);
}

void TaskKeysPage::on_new_task_action() {
  auto new_task_form = new LineInputForm(tr("New Task"), *this);
  new_task_form->setAttribute(Qt::WA_DeleteOnClose);
  new_task_form->connect_submit_signal(
    std::bind_front(&TaskKeysPage::on_new_task_submission, this));
  new_task_form->show();
  auto window = new_task_form->window();
  window->move(
    mapToGlobal(QPoint(0, 0)) + rect().center() - window->rect().center());
}

void TaskKeysPage::on_duplicate_task_action() {
  auto& selection = m_table_view->get_selection()->get_row_selection();
  if(selection->get_size() == 0) {
    return;
  }
  auto last_current_row = m_table_view->get_current()->get()->m_row;
  auto sorted_selection =
    std::vector<int>(selection->begin(), selection->end());
  std::sort(sorted_selection.begin(), sorted_selection.end(),
    std::greater<int>());
  for(auto index : sorted_selection) {
    auto order_task = m_key_bindings->get_order_task_arguments()->get(
      any_cast<int>(m_table_view->get_body().get_table()->at(index, 0)));
    order_task.m_key = QKeySequence();
    m_key_bindings->get_order_task_arguments()->insert(order_task,
      m_table_view->get_current()->get()->m_row);
  }
  QTimer::singleShot(0, this, [=] {
    m_table_view->get_current()->set(TableView::Index(last_current_row, 1));
  });
}

void TaskKeysPage::on_delete_task_action() {
  m_key_bindings->get_order_task_arguments()->transact([&] {
    for(auto i : *m_table_view->get_selection()->get_row_selection()) {
      m_key_bindings->get_order_task_arguments()->remove(
        any_cast<int>(m_table_view->get_body().get_table()->at(i, 0)));
    }
  });
}

void TaskKeysPage::on_new_task_submission(const QString& name) {
  auto order_task = OrderTaskArguments();
  order_task.m_name = name;
  m_is_row_added = true;
  if(auto& current = m_table_view->get_current()->get()) {
    m_key_bindings->get_order_task_arguments()->insert(order_task,
      current->m_row);
  } else {
    m_key_bindings->get_order_task_arguments()->push(order_task);
  }
  m_is_row_added = false;
}

void TaskKeysPage::on_row_selection(
    const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<int>::AddOperation& operation) {
      update_button_state();
    },
    [&] (const ListModel<int>::RemoveOperation& operation) {
      update_button_state();
    });
}

void TaskKeysPage::on_table_operation(const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      if(m_is_row_added) {
        QTimer::singleShot(0, this, [=] {
          auto index = TableView::Index(operation.m_index, 2);
          m_table_view->get_current()->set(index);
          m_added_region_item = m_table_view->get_body().get_item(index);
          m_added_region_item->installEventFilter(this);
        });
      }
    });
}
