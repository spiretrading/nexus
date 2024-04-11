#include "Spire/KeyBindings/TaskKeysTableView.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ColumnViewListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"
#include "Spire/Spire/ValidatedValueModel.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/DestinationBox.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/EditableTableView.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/EmptyTableFilter.hpp"
#include "Spire/Ui/KeyInputBox.hpp"
#include "Spire/Ui/OrderTypeBox.hpp"
#include "Spire/Ui/PopupBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/RegionBox.hpp"
#include "Spire/Ui/SideBox.hpp"
#include "Spire/Ui/SingleSelectionModel.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TimeInForceBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  enum class Column {
    NAME,
    REGION,
    DESTINATION,
    ORDER_TYPE,
    SIDE,
    QUANTITY,
    TIME_IN_FORCE,
    TAGS,
    KEY
  };

  auto to_list(const OrderTaskArguments& arguments) {
    auto list_model = std::make_shared<ArrayListModel<std::any>>();
    list_model->push(arguments.m_name);
    list_model->push(arguments.m_region);
    list_model->push(arguments.m_destination);
    list_model->push(arguments.m_order_type);
    list_model->push(arguments.m_side);
    list_model->push(arguments.m_quantity);
    list_model->push(arguments.m_time_in_force);
    list_model->push(arguments.m_additional_tags);
    list_model->push(arguments.m_key);
    return list_model;
  }

  auto make_quantity_modifiers() {
    return QHash<Qt::KeyboardModifier, Quantity>(
      {{Qt::NoModifier, 1}, {Qt::AltModifier, 5}, {Qt::ControlModifier, 10},
       {Qt::ShiftModifier, 20}});
  }

  auto key_input_box_validator(const QKeySequence& sequence) {
    if(sequence.count() == 0) {
      return QValidator::Intermediate;
    } else if(sequence.count() > 1) {
      return QValidator::Invalid;
    }
    auto key = sequence[0];
    auto modifier = key & Qt::KeyboardModifierMask;
    auto new_key = key - modifier;
    if(((modifier == Qt::NoModifier || modifier & Qt::ControlModifier ||
          modifier & Qt::ShiftModifier || modifier & Qt::AltModifier) &&
          new_key >= Qt::Key_F1 && new_key <= Qt::Key_F12) ||
        ((modifier & Qt::ControlModifier || modifier & Qt::ShiftModifier ||
          modifier & Qt::AltModifier) && new_key >= Qt::Key_0 &&
          new_key <= Qt::Key_9)) {
      return QValidator::Acceptable;
    }
    return QValidator::Invalid;
  }

  auto make_header_model() {
    auto model = std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
    auto push = [&] (const QString& name, const QString& short_name) {
      model->push({name, short_name, TableHeaderItem::Order::NONE,
        TableFilter::Filter::UNFILTERED});
    };
    push(QObject::tr("Name"), QObject::tr("Name"));
    push(QObject::tr("Region"), QObject::tr("Region"));
    push(QObject::tr("Destination"), QObject::tr("Dest"));
    push(QObject::tr("Order Type"), QObject::tr("Ord Type"));
    push(QObject::tr("Side"), QObject::tr("Side"));
    push(QObject::tr("Quantity"), QObject::tr("Qty"));
    push(QObject::tr("Time in Force"), QObject::tr("TIF"));
    push(QObject::tr("Tags"), QObject::tr("Tags"));
    push(QObject::tr("Key"), QObject::tr("Key"));
    return model;
  }

  auto make_header_widths() {
    auto widths = std::vector<int>();
    widths.push_back(scale_width(160));
    widths.push_back(scale_width(80));
    widths.push_back(scale_width(110));
    widths.push_back(scale_width(90));
    widths.push_back(scale_width(70));
    widths.push_back(scale_width(70));
    widths.push_back(scale_width(70));
    widths.push_back(scale_width(80));
    widths.push_back(scale_width(128));
    return widths;
  }

  AnyRef extract_field(const OrderTaskArguments& arguments, Column column) {
    if(column == Column::NAME) {
      return arguments.m_name;
    } else if(column == Column::REGION) {
      return arguments.m_region;
    } else if(column == Column::DESTINATION) {
      return arguments.m_destination;
    } else if(column == Column::ORDER_TYPE) {
      return arguments.m_order_type;
    } else if(column == Column::SIDE) {
      return arguments.m_side;
    } else if(column == Column::QUANTITY) {
      return arguments.m_quantity;
    } else if(column == Column::TIME_IN_FORCE) {
      return arguments.m_time_in_force;
    } else if(column == Column::TAGS) {
      return arguments.m_additional_tags;
    } else {
      return arguments.m_key;
    }
  }

  template<typename T>
  auto to_value_model(const std::shared_ptr<TableModel>& table, int row,
      int column) {
    return make_list_value_model(
      std::make_shared<ColumnViewListModel<T>>(table, column), row);
  }
}

struct DestinationQueryModel : ComboBox::QueryModel {
  std::shared_ptr<ValueModel<Region>> m_region_model;
  DestinationDatabase m_destinations;
  MarketDatabase m_markets;
  optional<LocalComboBoxQueryModel> m_local_query_model;
  scoped_connection m_region_connection;

  DestinationQueryModel(std::shared_ptr<ValueModel<Region>> region_model,
      DestinationDatabase destinations, MarketDatabase markets)
      : m_region_model(std::move(region_model)),
        m_destinations(std::move(destinations)),
        m_markets(std::move(markets)),
        m_region_connection(m_region_model->connect_update_signal(
          std::bind_front(&DestinationQueryModel::on_update, this))) {
    on_update(m_region_model->get());
  }

  std::any parse(const QString& query) override {
    return m_local_query_model->parse(query);
  }

  QtPromise<std::vector<std::any>> submit(const QString& query) override {
    return m_local_query_model->submit(query);
  }

  void on_update(const Region& region) {
    auto market_set = std::unordered_set<MarketCode>();
    for(auto& security : region.GetSecurities()) {
      market_set.insert(security.GetMarket());
    }
    auto region_markets = region.GetMarkets();
    market_set.insert(region_markets.begin(), region_markets.end());
    for(auto& country : region.GetCountries()) {
      auto markets = m_markets.FromCountry(country);
      for(auto& market : markets) {
        market_set.insert(market.m_code);
      }
    }
    m_local_query_model.emplace();
    auto destinations = m_destinations.SelectEntries(
      [] (auto& value) { return true; });
    for(auto& destination : destinations) {
      for(auto& market : destination.m_markets) {
        if(market_set.contains(market)) {
          m_local_query_model->add(to_text(destination.m_id).toLower(),
            destination);
          break;
        }
      }
    }
  }
};

struct DestinationValueModel : ValueModel<Destination> {
  mutable UpdateSignal m_update_signal;
  std::shared_ptr<ValueModel<Destination>> m_source;
  std::shared_ptr<DestinationQueryModel> m_query_model;
  scoped_connection m_source_connection;
  scoped_connection m_connection;

  DestinationValueModel(std::shared_ptr<ValueModel<Destination>> source,
      std::shared_ptr<DestinationQueryModel> query_model)
      : m_source(std::move(source)),
        m_query_model(std::move(query_model)),
        m_source_connection(m_source->connect_update_signal(m_update_signal)),
        m_connection(m_query_model->m_region_model->connect_update_signal(
          std::bind_front(&DestinationValueModel::on_update, this))) {
    on_update(m_query_model->m_region_model->get());
  }

  QValidator::State get_state() const override {
    return m_source->get_state();
  }

  const Destination& get() const override {
    return m_source->get();
  }

  QValidator::State test(const Destination& value) const override {
    return m_source->test(value);
  }

  QValidator::State set(const Destination& value) override {
    return m_source->set(value);
  }

  connection connect_update_signal(
      const UpdateSignal::slot_type& slot) const override {
    return m_update_signal.connect(slot);
  }

  void on_update(const Region& region) {
    if(!get().empty() && !m_query_model->parse(to_text(get())).has_value()) {
      set(Destination());
    }
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
    return extract_field(m_source->get(row), static_cast<Column>(column));
  }

  QValidator::State set(int row, int column, const std::any& value) override {
    if(column < 0 || column >= get_column_size()) {
      throw std::out_of_range("The column is out of range.");
    }
    auto column_index = static_cast<Column>(column);
    auto arguments = m_source->get(row);
    auto previous = to_any(extract_field(arguments, column_index));
    if(column_index == Column::NAME) {
      arguments.m_name = std::any_cast<const QString&>(value);
    } else if(column_index == Column::REGION) {
      arguments.m_region = std::any_cast<const Region&>(value);
    } else if(column_index == Column::DESTINATION) {
      arguments.m_destination = std::any_cast<const Destination&>(value);
    } else if(column_index == Column::ORDER_TYPE) {
      arguments.m_order_type = std::any_cast<const OrderType&>(value);
    } else if(column_index == Column::SIDE) {
      arguments.m_side = std::any_cast<const Side&>(value);
    } else if(column_index == Column::QUANTITY) {
      arguments.m_quantity = std::any_cast<const optional<Quantity>&>(value);
    } else if(column_index == Column::TIME_IN_FORCE) {
      arguments.m_time_in_force = std::any_cast<const TimeInForce&>(value);
    } else if(column_index == Column::TAGS) {
      arguments.m_additional_tags =
        std::any_cast<const std::vector<Nexus::Tag>&>(value);
    } else if(column_index == Column::KEY) {
      arguments.m_key = std::any_cast<const QKeySequence&>(value);
    }
    auto blocker = shared_connection_block(m_source_connection);
    auto result = m_source->set(row, std::move(arguments));
    if(result == QValidator::State::Invalid) {
      return QValidator::State::Invalid;
    }
    m_transaction.push(
      TableModel::UpdateOperation(row, column, previous, value));
    return result;
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
        m_transaction.push(TableModel::AddOperation(operation.m_index,
          to_list(operation.get_value())));
      },
      [&] (const OrderTaskArgumentsListModel::MoveOperation& operation) {
        m_transaction.push(TableModel::MoveOperation(
          operation.m_source, operation.m_destination));
      },
      [&] (const OrderTaskArgumentsListModel::RemoveOperation& operation) {
        m_transaction.push(TableModel::RemoveOperation(operation.m_index,
          to_list(operation.get_value())));
      },
      [&] (const OrderTaskArgumentsListModel::UpdateOperation& operation) {
        m_transaction.transact([&] {
          for(auto i = 0; i < COLUMN_SIZE; ++i) {
            m_transaction.push(TableModel::UpdateOperation(operation.m_index, i,
              to_any(extract_field(operation.get_previous(),
                static_cast<Column>(i))),
              to_any(extract_field(operation.get_value(),
                static_cast<Column>(i)))));
          }
        });
      });
  }
};

EditableBox* make_table_item(
    std::shared_ptr<ComboBox::QueryModel> region_query_model,
    const DestinationDatabase& destinations, const MarketDatabase& markets,
    const std::shared_ptr<TableModel>& table, int row, int column) {
  auto column_id = static_cast<Column>(column);
  auto input_box = [&] {
    if(column_id == Column::NAME) {
      return new AnyInputBox(*new TextBox(
        to_value_model<QString>(table, row, column)));
    } else if(column_id == Column::REGION) {
      return new AnyInputBox(*new RegionBox(region_query_model,
        to_value_model<Region>(table, row, column)));
    } else if(column_id == Column::DESTINATION) {
      auto region_model = to_value_model<Region>(table, row,
        static_cast<int>(Column::REGION));
      auto query_model = std::make_shared<DestinationQueryModel>(
        std::move(region_model), destinations, markets);
      auto current_model = std::make_shared<DestinationValueModel>(
        to_value_model<Destination>(table, row, column), query_model);
      return new AnyInputBox(*new DestinationBox(std::move(query_model),
        std::move(current_model)));
    } else if(column_id == Column::ORDER_TYPE) {
      return new AnyInputBox(*make_order_type_box(
        to_value_model<OrderType>(table, row, column)));
    } else if(column_id == Column::SIDE) {
      return new AnyInputBox(*make_side_box(
        to_value_model<Side>(table, row, column)));
    } else if(column_id == Column::QUANTITY) {
      return new AnyInputBox(*new QuantityBox(
        std::make_shared<ScalarValueModelDecorator<optional<Quantity>>>(
          to_value_model<optional<Quantity>>(table, row, column)),
        make_quantity_modifiers()));
    } else if(column_id == Column::TIME_IN_FORCE) {
      return new AnyInputBox(*make_time_in_force_box(
        to_value_model<TimeInForce>(table, row, column)));
    } else if(column_id == Column::TAGS) {
      return new AnyInputBox(*make_label(""));
    } else {
      return new AnyInputBox(*new KeyInputBox(
        make_validated_value_model<QKeySequence>(&key_input_box_validator,
          to_value_model<QKeySequence>(table, row, column))));
    } 
  }();
  if(column_id == Column::KEY) {
    return new EditableBox(*input_box, [] (const auto& key) {
      return key_input_box_validator(key) != QValidator::Invalid;
    });
  }
  return new EditableBox(*input_box);
}

TableView* Spire::make_task_keys_table_view(
    std::shared_ptr<OrderTaskArgumentsListModel> order_task_arguments,
    std::shared_ptr<ComboBox::QueryModel> region_query_model,
    Nexus::DestinationDatabase destinations, Nexus::MarketDatabase markets,
    QWidget* parent) {
  auto table_view = new EditableTableView(
    std::make_shared<OrderTaskTableModel>(order_task_arguments),
    make_header_model(), std::make_shared<EmptyTableFilter>(),
    std::make_shared<LocalValueModel<optional<TableIndex>>>(),
    std::make_shared<TableSelectionModel>(
      std::make_shared<TableEmptySelectionModel>(),
      std::make_shared<ListSingleSelectionModel>(),
      std::make_shared<ListEmptySelectionModel>()),
    std::bind_front(&make_table_item, region_query_model, destinations,
      markets), {});
  auto widths = make_header_widths();
  for(auto i = 0; i < std::ssize(widths); ++i) {
    table_view->get_header().get_widths()->set(i + 1, widths[i]);
  }
  return table_view;
}
