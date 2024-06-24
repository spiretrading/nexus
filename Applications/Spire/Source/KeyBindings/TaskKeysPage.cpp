#include "Spire/KeyBindings/TaskKeysPage.hpp"
#include "Spire/KeyBindings/AdditionalTag.hpp"
#include "Spire/KeyBindings/OrderTaskArgumentsMatch.hpp"
#include "Spire/KeyBindings/TaskKeysTableView.hpp"
#include "Spire/Spire/FilteredTableModel.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
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

  AnyRef extract_field(const OrderTaskArguments& arguments,
      OrderTaskColumns column) {
    if(column == OrderTaskColumns::NAME) {
      return arguments.m_name;
    } else if(column == OrderTaskColumns::REGION) {
      return arguments.m_region;
    } else if(column == OrderTaskColumns::DESTINATION) {
      return arguments.m_destination;
    } else if(column == OrderTaskColumns::ORDER_TYPE) {
      return arguments.m_order_type;
    } else if(column == OrderTaskColumns::SIDE) {
      return arguments.m_side;
    } else if(column == OrderTaskColumns::QUANTITY) {
      return arguments.m_quantity;
    } else if(column == OrderTaskColumns::TIME_IN_FORCE) {
      return arguments.m_time_in_force;
    } else if(column == OrderTaskColumns::TAGS) {
      return arguments.m_additional_tags;
    } else {
      return arguments.m_key;
    }
  }

  Region make_region(const SecurityInfo& security_info) {
    auto region = Region(security_info.m_security);
    region.SetName(security_info.m_name);
    return region;
  }

  auto populate_region_query_model(const CountryDatabase& countries,
      const MarketDatabase& markets) {
    auto model = std::make_shared<LocalComboBoxQueryModel>();
    for(auto& country : countries.GetEntries()) {
      auto region = Region(country.m_code);
      region.SetName(country.m_name);
      model->add(to_text(country.m_code).toLower(), region);
      model->add(QString::fromStdString(region.GetName()).toLower(), region);
    }
    for(auto& market : markets.GetEntries()) {
      auto region = Region(market);
      region.SetName(market.m_description);
      model->add(to_text(MarketToken(market.m_code)).toLower(), region);
      model->add(QString::fromStdString(region.GetName()).toLower(), region);
    }
    return model;
  }

  struct OrderTaskArgumentsMatchCache {
    std::unordered_set<QString> m_caches;

    bool matches(const OrderTaskArguments& order_task, const QString& query,
        const CountryDatabase& countries, const MarketDatabase& markets,
        const DestinationDatabase& destinations) {
      if(m_caches.contains(query)) {
        return true;
      }
      auto matched = ::matches(order_task, query, countries, markets,
        destinations);
      if(matched) {
        m_caches.insert(query);
      }
      return matched;
    }
  };

  struct OrderTaskTableModel : TableModel {
    static const auto COLUMN_SIZE = 9;
    std::shared_ptr<OrderTaskArgumentsListModel> m_source;
    TableModelTransactionLog m_transaction;
    scoped_connection m_source_connection;

    explicit OrderTaskTableModel(
      std::shared_ptr<OrderTaskArgumentsListModel> source)
      : m_source(std::move(source)),
        m_source_connection(m_source->connect_operation_signal(
          std::bind_front(&OrderTaskTableModel::on_operation, this))) {}

    int get_row_size() const override {
      return m_source->get_size();
    }

    int get_column_size() const override {
      return COLUMN_SIZE;
    }

    AnyRef at(int row, int column) const override {
      if(column < 0 || column >= get_column_size()) {
        throw std::out_of_range("The column is out of range.");
      }
      return extract_field(
        m_source->get(row), static_cast<OrderTaskColumns>(column));
    }

    QValidator::State set(int row, int column, const std::any& value) override {
      if(column < 0 || column >= get_column_size()) {
        throw std::out_of_range("The column is out of range.");
      }
      auto column_index = static_cast<OrderTaskColumns>(column);
      auto arguments = m_source->get(row);
      if(column_index == OrderTaskColumns::NAME) {
        arguments.m_name = std::any_cast<const QString&>(value);
      } else if(column_index == OrderTaskColumns::REGION) {
        arguments.m_region = std::any_cast<const Region&>(value);
      } else if(column_index == OrderTaskColumns::DESTINATION) {
        arguments.m_destination = std::any_cast<const Destination&>(value);
      } else if(column_index == OrderTaskColumns::ORDER_TYPE) {
        arguments.m_order_type = std::any_cast<OrderType>(value);
      } else if(column_index == OrderTaskColumns::SIDE) {
        arguments.m_side = std::any_cast<Side>(value);
      } else if(column_index == OrderTaskColumns::QUANTITY) {
        arguments.m_quantity = std::any_cast<QuantitySetting>(value);
      } else if(column_index == OrderTaskColumns::TIME_IN_FORCE) {
        arguments.m_time_in_force = std::any_cast<const TimeInForce&>(value);
      } else if(column_index == OrderTaskColumns::TAGS) {
        arguments.m_additional_tags =
          std::any_cast<const std::vector<AdditionalTag>&>(value);
      } else if(column_index == OrderTaskColumns::KEY) {
        arguments.m_key = std::any_cast<const QKeySequence&>(value);
      }
      return m_source->set(row, std::move(arguments));
    }

    QValidator::State remove(int row) override {
      return m_source->remove(row);
    }

    connection connect_operation_signal(
      const OperationSignal::slot_type& slot) const override {
      return m_transaction.connect_operation_signal(slot);
    }

    void on_operation(const OrderTaskArgumentsListModel::Operation& operation) {
      visit(operation,
        [&] (const StartTransaction&) {
          m_transaction.start();
        },
        [&] (const EndTransaction&) {
          m_transaction.end();
        },
        [&] (const OrderTaskArgumentsListModel::AddOperation& operation) {
          m_transaction.push(TableModel::AddOperation(operation.m_index));
        },
        [&] (const OrderTaskArgumentsListModel::MoveOperation& operation) {
          m_transaction.push(TableModel::MoveOperation(
            operation.m_source, operation.m_destination));
        },
        [&] (const OrderTaskArgumentsListModel::PreRemoveOperation& operation) {
          m_transaction.push(TableModel::PreRemoveOperation(operation.m_index));
        },
        [&] (const OrderTaskArgumentsListModel::RemoveOperation& operation) {
          m_transaction.push(TableModel::RemoveOperation(operation.m_index));
        },
        [&] (const OrderTaskArgumentsListModel::UpdateOperation& operation) {
          m_transaction.transact([&] {
            for(auto i = 0; i < COLUMN_SIZE; ++i) {
              m_transaction.push(TableModel::UpdateOperation(operation.m_index,
                i, to_any(extract_field(operation.get_previous(),
                  static_cast<OrderTaskColumns>(i))),
                to_any(extract_field(operation.get_value(),
                  static_cast<OrderTaskColumns>(i)))));
            }
          });
      });
    }
  };

  struct RegionQueryModel : ComboBox::QueryModel {
    std::shared_ptr<ComboBox::QueryModel> m_securities;
    std::shared_ptr<ComboBox::QueryModel> m_regions;

    RegionQueryModel(std::shared_ptr<ComboBox::QueryModel> securities,
      std::shared_ptr<ComboBox::QueryModel> regions)
      : m_securities(std::move(securities)),
        m_regions(std::move(regions)) {}

    std::any parse(const QString& query) override {
      auto security = m_securities->parse(query);
      if(security.has_value()) {
        return make_region(std::any_cast<SecurityInfo&>(security));
      }
      return m_regions->parse(query);
    }

    QtPromise<std::vector<std::any>> submit(const QString& query) override {
      return m_securities->submit(query).then([=] (auto&& security_result) {
        auto security_matches = [&] {
          try {
            return security_result.Get();
          } catch(const std::exception&) {
            return std::vector<std::any>();
          }
        }();
        auto result = std::vector<std::any>();
        auto region_set = std::unordered_set<Region>();
        for(auto& security : security_matches) {
          auto region = make_region(std::any_cast<SecurityInfo&>(security));
          if(region_set.insert(region).second) {
            result.push_back(region);
          }
        }
        return m_regions->submit(query).then(
          [=] (auto&& region_result) mutable {
            auto region_matches = [&] {
              try {
                return region_result.Get();
              } catch(const std::exception&) {
                return std::vector<std::any>();
              }
            }();
            for(auto& region : region_matches) {
              if(region_set.insert(std::any_cast<Region&>(region)).second) {
                result.push_back(region);
              }
            }
            return result;
          });
      });
    }
  };
}

struct TaskKeysPage::OrderTaskMatchCache {
  std::shared_ptr<OrderTaskArgumentsListModel> m_source;
  MarketDatabase m_markets;
  CountryDatabase m_countries;
  DestinationDatabase m_destinations;
  std::vector<OrderTaskArgumentsMatchCache> m_caches;
  scoped_connection m_connection;

  OrderTaskMatchCache(std::shared_ptr<OrderTaskArgumentsListModel> source,
    CountryDatabase countries, MarketDatabase markets,
    DestinationDatabase destinations)
    : m_source(std::move(source)),
      m_countries(std::move(countries)),
      m_markets(std::move(markets)),
      m_destinations(std::move(destinations)),
      m_caches(m_source->get_size()),
      m_connection(m_source->connect_operation_signal(
        std::bind_front(&OrderTaskMatchCache::on_operation, this))) {}

  bool matches(int row, const QString& query) {
    if(query.isEmpty()) {
      return true;
    }
    return m_caches[row].matches(m_source->get(row), query,
      m_countries, m_markets, m_destinations);
  }

  void on_operation(const OrderTaskArgumentsListModel::Operation& operation) {
    visit(operation,
      [&] (const OrderTaskArgumentsListModel::AddOperation& operation) {
        m_caches.insert(std::next(m_caches.begin(), operation.m_index),
          OrderTaskArgumentsMatchCache());
      },
      [&] (const OrderTaskArgumentsListModel::RemoveOperation& operation) {
        m_caches.erase(std::next(m_caches.begin(), operation.m_index));
      },
      [&] (const OrderTaskArgumentsListModel::UpdateOperation& operation) {
        m_caches[operation.m_index] = OrderTaskArgumentsMatchCache();
      });
  }
};

TaskKeysPage::TaskKeysPage(std::shared_ptr<KeyBindingsModel> key_bindings,
    std::shared_ptr<ComboBox::QueryModel> securities,
    CountryDatabase countries, MarketDatabase markets,
    DestinationDatabase destinations, AdditionalTagDatabase additional_tags,
    QWidget* parent)
    : QWidget(parent),
      m_key_bindings(std::move(key_bindings)),
      m_match_cache(std::make_unique<OrderTaskMatchCache>(
        m_key_bindings->get_order_task_arguments(),
        std::move(countries), std::move(markets), std::move(destinations))),
      m_filtered_model(std::make_shared<FilteredTableModel>(
        std::make_shared<OrderTaskTableModel>(m_match_cache->m_source),
        std::bind_front(&TaskKeysPage::filter, this, ""))),
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
    style.get((Hover() || Press()) > is_a<Icon>()).set(Fill(QColor(0xB71C1C)));
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
    m_filtered_model, std::make_shared<RegionQueryModel>(std::move(securities),
      populate_region_query_model(m_match_cache->m_countries,
        m_match_cache->m_markets)),
    m_match_cache->m_destinations, m_match_cache->m_markets, additional_tags);
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
        &m_added_region_item->get_body())->set_read_only(false);
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

bool TaskKeysPage::filter(const QString& query, const TableModel&, int row) {
  return !m_match_cache->matches(row, query);
}

void TaskKeysPage::update_button_state() {
  auto is_enabled =
    m_table_view->get_selection()->get_row_selection()->get_size() > 0;
  m_duplicate_button->setEnabled(is_enabled);
  m_delete_button->setEnabled(is_enabled);
}

void TaskKeysPage::on_search(const QString& query) {
  m_filtered_model->set_filter(
    std::bind_front(&TaskKeysPage::filter, this, query));
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
