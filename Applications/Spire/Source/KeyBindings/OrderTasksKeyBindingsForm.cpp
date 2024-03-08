#include "Spire/KeyBindings/OrderTasksKeyBindingsForm.hpp"
#include "Spire/KeyBindings/OrderTaskArgumentsMatch.hpp"
#include "Spire/KeyBindings/TableMatchCache.hpp"
#include "Spire/LegacyUI/HashQtTypes.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ColumnViewListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Spire/ValidatedValueModel.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/DestinationBox.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/EditableTableView.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/EmptyTableFilter.hpp"
#include "Spire/Ui/KeyInputBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/OrderTypeBox.hpp"
#include "Spire/Ui/PopupBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/RegionBox.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/SearchBox.hpp"
#include "Spire/Ui/SideBox.hpp"
#include "Spire/Ui/SingleSelectionModel.hpp"
#include "Spire/Ui/TextAreaBox.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TimeInForceBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto make_quantity_modifiers() {
    return QHash<Qt::KeyboardModifier, Quantity>(
      {{Qt::NoModifier, 1}, {Qt::AltModifier, 5}, {Qt::ControlModifier, 10},
       {Qt::ShiftModifier, 20}});
  }

  auto test_key_sequence(const QKeySequence& sequence) {
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

  bool compare_text(const QString& lhs, const QString& rhs) {
    return QString::compare(lhs, rhs, Qt::CaseInsensitive) < 0;
  }

  bool order_task_argument_comparator(const AnyRef& lhs, const AnyRef& rhs) {
    if(lhs.get_type() != rhs.get_type()) {
      return false;
    } else if(!lhs.has_value() || !rhs.has_value()) {
      return false;
    } else if(lhs.get_type() == typeid(QString)) {
      return compare_text(any_cast<QString>(lhs), any_cast<QString>(rhs));
    } else if(lhs.get_type() == typeid(optional<Quantity>)) {
      auto& quantity_lhs = any_cast<optional<Quantity>>(lhs);
      auto& quantity_rhs = any_cast<optional<Quantity>>(rhs);
      if(quantity_lhs && quantity_rhs) {
        return *quantity_lhs < *quantity_rhs;
      } else if(quantity_lhs) {
        return false;
      } else if(quantity_rhs) {
        return true;
      }
    } else if(lhs.get_type() == typeid(Side)) {
      auto& side_lhs = any_cast<Side>(lhs);
      auto& side_rhs = any_cast<Side>(rhs);
      if(side_lhs != Side::NONE && side_rhs != Side::NONE) {
        return to_text(side_lhs) < to_text(side_rhs);
      } else if(side_lhs != Side::NONE) {
        return false;
      } else if(side_rhs != Side::NONE) {
        return true;
      }
    } else if(lhs.get_type() == typeid(TimeInForce)) {
      return any_cast<TimeInForce>(lhs).GetType() <
        any_cast<TimeInForce>(rhs).GetType();
    } else if(lhs.get_type() == typeid(OrderType)) {
      return any_cast<OrderType>(lhs) < any_cast<OrderType>(rhs);
    } else if(lhs.get_type() == typeid(Destination)) {
      return any_cast<Destination>(lhs) < any_cast<Destination>(rhs);
    } else if(lhs.get_type() == typeid(Region)) {
      return compare_text(to_text(any_cast<Region>(lhs)),
        to_text(any_cast<Region>(rhs)));
    } else if(lhs.get_type() == typeid(QKeySequence)) {
      return compare_text(any_cast<QKeySequence>(lhs).toString(),
        any_cast<QKeySequence>(rhs).toString());
    }
    return false;
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
    widths.push_back(0);
    widths.push_back(scale_width(128));
    return widths;
  }

  auto make_header_model() {
    auto model = std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
    model->push({"Name", "Name",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    model->push({"Region", "Region",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    model->push({"Destination", "Dest",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    model->push({"Order Type", "Ord Type",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    model->push({"Side", "Side",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    model->push({"Quantity", "Qty",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    model->push({"Time in Force", "TIF",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    model->push({"Tags", "Tags",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    model->push({"Key", "Key",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    return model;
  }

  template<typename T>
  auto make_matcher(const std::shared_ptr<TableModel>& table, int row,
    int column) {
    return [=] (const QString& query) {
      return matches(table->get<T>(row, column), query);
    };
  }

  TableMatchCache::Matcher table_matcher_builder(
      const std::shared_ptr<TableModel>& table, int row, int column) {
    auto column_id = static_cast<OrderTaskArgumentsToTableModel::Column>(column);
    if(column_id == OrderTaskArgumentsToTableModel::Column::NAME) {
      return make_matcher<QString>(table, row, column);
    } else if(column_id == OrderTaskArgumentsToTableModel::Column::REGION) {
      return make_matcher<Region>(table, row, column);
    } else if(column_id == OrderTaskArgumentsToTableModel::Column::DESTINATION) {
      return make_matcher<Destination>(table, row, column);
    } else if(column_id == OrderTaskArgumentsToTableModel::Column::ORDER_TYPE) {
      return make_matcher<OrderType>(table, row, column);
    } else if(column_id == OrderTaskArgumentsToTableModel::Column::SIDE) {
      return make_matcher<Side>(table, row, column);
    } else if(column_id == OrderTaskArgumentsToTableModel::Column::QUANTITY) {
      return make_matcher<optional<Quantity>>(table, row, column);
    } else if(column_id == OrderTaskArgumentsToTableModel::Column::TIME_IN_FORCE) {
      return make_matcher<TimeInForce>(table, row, column);
    } else if(column_id == OrderTaskArgumentsToTableModel::Column::KEY) {
      return make_matcher<QKeySequence>(table, row, column);
    }
    return [] (const QString& query) { return false; };
  }

  auto make_help_text_box() {
    auto help_text_box = make_text_area_label(
      "Allowed keys are: <b>F1–F12</b> and <b>Ctrl, Shift, Alt + F1–F12</b> "
      "and <b>Ctrl, Shift, Alt + 0–9</b>");
    help_text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    update_style(*help_text_box, [] (auto& style) {
      auto font = QFont("Roboto");
      font.setWeight(QFont::Normal);
      font.setPixelSize(scale_width(10));
      style.get(Any()).
        set(BackgroundColor(QColor(0xFFFFFF))).
        set(text_style(font, QColor(0x808080))).
        set(horizontal_padding(scale_width(8))).
        set(PaddingTop(scale_height(10))).
        set(PaddingBottom(scale_height(5)));
    });
    return help_text_box;
  }

  auto make_search_region() {
    auto search_box = new SearchBox();
    search_box->setFocusPolicy(Qt::StrongFocus);
    search_box->set_placeholder(QObject::tr("Search tasks"));
    search_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto search_region = new Box(search_box);
    search_region->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Preferred);
    update_style(*search_region, [] (auto& style) {
      style.get(Any()).
        set(BackgroundColor(QColor(0xFFFFFF))).
        set(horizontal_padding(scale_width(8))).
        set(PaddingTop(scale_height(5))).
        set(PaddingBottom(scale_height(10))).
        set(BorderBottomSize(scale_height(1))).
        set(BorderBottomColor(QColor(0xE0E0E0)));
    });
    return std::tuple(search_region, search_box);
  }

  template<typename T>
  class CustomListValueModel : public ListValueModel<T> {
    public:
      using Type = typename ListValueModel<T>::Type;

      CustomListValueModel(std::shared_ptr<ListModel<Type>> source, int index)
        : ListValueModel<Type>(std::move(source), index) {}

      const Type& get() const override {
        if(ListValueModel<Type>::get_state() == QValidator::State::Acceptable) {
          return ListValueModel<Type>::get();
        }
        static auto value = Type();
        return value;
      }
  };

  template<typename T>
  auto make_custom_list_value_model(std::shared_ptr<T> source, int index) {
    return std::make_shared<CustomListValueModel<typename T::Type>>(
      std::move(source), index);
  }

  template<typename T>
  class CustomColumnViewListModel : public ColumnViewListModel<T> {
    public:
      using Type = typename ColumnViewListModel<T>::Type;

      CustomColumnViewListModel(std::shared_ptr<TableModel> source, int column)
        : ColumnViewListModel<T>(std::move(source), column) {}

      const Type& get(int index) const override {
        if(index != ColumnViewListModel<Type>::get_size() - 1) {
          return ColumnViewListModel<Type>::get(index);
        }
        static auto value = Type();
        return value;
      }
  };

  struct DestinationQueryModel : ComboBox::QueryModel {
    std::shared_ptr<ValueModel<Region>> m_region_model;
    DestinationDatabase m_destinations;
    MarketDatabase m_markets;
    std::unique_ptr<LocalComboBoxQueryModel> m_local_query_model;
    scoped_connection m_region_connection;

    DestinationQueryModel(std::shared_ptr<ValueModel<Region>> region_model,
        DestinationDatabase destinations, MarketDatabase markets)
        : m_region_model(std::move(region_model)),
          m_destinations(std::move(destinations)),
          m_markets(std::move(markets)),
          m_local_query_model(std::make_unique<LocalComboBoxQueryModel>()),
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
      auto destinations = m_destinations.SelectEntries(
        [] (auto& value) { return true; });
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
      m_local_query_model = std::make_unique<LocalComboBoxQueryModel>();
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
    std::shared_ptr<ValueModel<Destination>> m_source;
    std::shared_ptr<DestinationQueryModel> m_query_model;
    scoped_connection m_connection;

    DestinationValueModel(std::shared_ptr<ValueModel<Destination>> source,
      std::shared_ptr<DestinationQueryModel> query_model)
      : m_source(std::move(source)),
        m_query_model(std::move(query_model)),
        m_connection(m_query_model->m_region_model->connect_update_signal(
          std::bind_front(&DestinationValueModel::on_update, this))) {}

    QValidator::State get_state() const override {
      return m_source->get_state();
    }

    const Type& get() const override {
      return m_source->get();
    }

    QValidator::State test(const Type& value) const override {
      return m_source->test(value);
    }

    QValidator::State set(const Type& value) override {
      return m_source->set(value);
    }

    connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override {
      return m_source->connect_update_signal(slot);
    }

    void on_update(const Region& region) {
      if(!get().empty() &&
          !m_query_model->parse(to_text(get())).has_value()) {
        set(Destination());
      }
    }
  };

  struct RegionKeyHash {
    std::size_t operator()(
        const std::pair<Region, QKeySequence>& region_key) const {
      auto seed = std::size_t(0);
      hash_combine(seed, hash_range(region_key.first.GetCountries().begin(),
        region_key.first.GetCountries().end()));
      auto markets = region_key.first.GetMarkets();
      hash_combine(seed, hash_range(markets.begin(), markets.end()));
      hash_combine(seed, hash_range(region_key.first.GetSecurities().begin(),
        region_key.first.GetSecurities().end()));
      hash_combine(seed, qHash(region_key.second));
      return seed;
    }
  };
}

struct OrderTasksKeyBindingsForm::UniqueKeyBindingsModel {
  struct UpdatedInfo {
    Region m_region;
    QKeySequence m_key;
    int m_row;
  };
  static const auto KeyIndex =
    static_cast<int>(OrderTaskArgumentsToTableModel::Column::KEY);
  static const auto RegionIndex =
    static_cast<int>(OrderTaskArgumentsToTableModel::Column::REGION);
  std::shared_ptr<TableModel> m_source;
  std::unordered_set<std::pair<Region, QKeySequence>, RegionKeyHash>
    m_region_key_set;
  UpdatedInfo m_updated_info;
  scoped_connection m_connection;

  UniqueKeyBindingsModel(std::shared_ptr<TableModel> source)
      : m_source(std::move(source)),
        m_connection(m_source->connect_operation_signal(
          std::bind_front(&UniqueKeyBindingsModel::on_operation, this))) {
    for(auto row = 0; row < m_source->get_row_size(); ++row) {
      if(auto& key = m_source->get<QKeySequence>(row, KeyIndex); !key.isEmpty()) {
        m_region_key_set.insert(std::pair(
          m_source->get<Region>(row, RegionIndex), key));
      }
    }
  }

  void update() {
    if(!m_region_key_set.contains(
        {m_updated_info.m_region, m_updated_info.m_key})) {
      return;
    }
    for(auto i = 0; i < m_source->get_row_size(); ++i) {
      if(i != m_updated_info.m_row) {
        if(m_source->get<QKeySequence>(i, KeyIndex) == m_updated_info.m_key &&
            m_source->get<Region>(i, RegionIndex) == m_updated_info.m_region) {
          m_source->set(i, KeyIndex, QKeySequence());
          return;
        }
      }
    }
  }

  void on_operation(const TableModel::Operation& operation) {
    visit(operation,
      [&] (const TableModel::AddOperation& operation) {
        auto key = std::any_cast<QKeySequence>(operation.m_row->get(KeyIndex));
        auto region = std::any_cast<Region>(operation.m_row->get(RegionIndex));
        if(!key.isEmpty()) {
          m_region_key_set.insert(std::pair(region, key));
          m_updated_info = {region, key, operation.m_index};
          update();
        }
      },
      [&] (const TableModel::RemoveOperation& operation) {
        m_region_key_set.erase({
          std::any_cast<Region>(operation.m_row->get(RegionIndex)),
          std::any_cast<QKeySequence>(operation.m_row->get(KeyIndex))});
      },
      [&] (const TableModel::UpdateOperation& operation) {
        if(operation.m_column == RegionIndex) {
          auto& region = std::any_cast<const Region&>(operation.m_value);
          auto& key = m_source->get<QKeySequence>(operation.m_row, KeyIndex);
          m_region_key_set.erase(
            {std::any_cast<Region>(operation.m_previous), key});
          if(!key.isEmpty()) {
            m_region_key_set.insert({region, key});
            m_updated_info = {region, key, operation.m_row};
          }
        } else if(operation.m_column == KeyIndex) {
          auto& region = m_source->get<Region>(operation.m_row, RegionIndex);
          auto& key = std::any_cast<const QKeySequence&>(operation.m_value);
          m_region_key_set.erase(
            {region, std::any_cast<QKeySequence>(operation.m_previous)});
          if(!key.isEmpty()) {
            m_region_key_set.insert({region, key});
            m_updated_info = {region, key, operation.m_row};
          }
        }
      });
  }
};

OrderTasksKeyBindingsForm::OrderTasksKeyBindingsForm(
    std::shared_ptr<ComboBox::QueryModel> region_query_model,
    std::shared_ptr<OrderTaskArgumentsListModel> order_task_arguments,
    Nexus::DestinationDatabase destinations, Nexus::MarketDatabase markets,
    QWidget* parent)
    : QWidget(parent),
      m_region_query_model(std::move(region_query_model)),
      m_order_task_arguments(std::move(order_task_arguments)),
      m_destinations(std::move(destinations)),
      m_markets(std::move(markets)) {
  auto layout = make_vbox_layout(this);
  layout->addWidget(make_help_text_box());
  auto [search_region, m_search_box] = make_search_region();
  m_search_box->connect_submit_signal(
    std::bind_front(&OrderTasksKeyBindingsForm::on_search, this));
  layout->addWidget(search_region);
  auto order_task_arguments_table =
    std::make_shared<OrderTaskArgumentsToTableModel>(m_order_task_arguments);
  m_unique_key_bindings =
    std::make_unique<UniqueKeyBindingsModel>(order_task_arguments_table);
  m_table_view = new EditableTableView(order_task_arguments_table,
    make_header_model(), std::make_shared<EmptyTableFilter>(),
    std::make_shared<LocalValueModel<optional<TableIndex>>>(),
    std::make_shared<TableSelectionModel>(
      std::make_shared<TableEmptySelectionModel>(),
      std::make_shared<ListSingleSelectionModel>(),
      std::make_shared<ListEmptySelectionModel>()),
    std::bind_front(&OrderTasksKeyBindingsForm::make_cell, this),
    order_task_argument_comparator,
    [] (const TableModel&, int) {
      return false;
    });
  m_table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto table_header = static_cast<TableHeader*>(static_cast<Box*>(
    m_table_view->layout()->itemAt(0)->widget())->get_body()->layout()->
      itemAt(0)->widget());
  auto widths = make_header_widths();
  for(auto i = 0; i < std::ssize(widths); ++i) {
    table_header->get_widths()->set(i + 1, widths[i]);
  }
  auto table_view_box = new Box(m_table_view);
  table_view_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*table_view_box, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
  });
  layout->addWidget(table_view_box);
  m_table_match_cache = std::make_unique<TableMatchCache>(
    order_task_arguments_table, table_matcher_builder);
  m_table_view->connect_delete_signal(
    std::bind_front(&OrderTasksKeyBindingsForm::on_delete, this));
}

const std::shared_ptr<ComboBox::QueryModel>&
    OrderTasksKeyBindingsForm::get_region_query_model() const {
  return m_region_query_model;
}

const std::shared_ptr<OrderTaskArgumentsListModel>&
    OrderTasksKeyBindingsForm::get_order_task_arguments() const {
  return m_order_task_arguments;
}

QWidget* OrderTasksKeyBindingsForm::make_cell(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  auto editor = [&] {
    if(row < table->get_row_size() - 1) {
      return make_editor(table, row, column);
    }
    return make_empty_editor(table, row, column);
  }();
  editor->setFocusPolicy(Qt::ClickFocus);
  update_style(*editor, [&] (auto& style) {
    switch(static_cast<Column>(column)) {
      case Column::ORDER_TYPE:
      case Column::SIDE:
      case Column::TIME_IN_FORCE:
        style.get(Any() > (is_a<TextBox>() && !(+Any() << is_a<ListItem>()))).
          set(horizontal_padding(scale_width(8)));
        break;
      case Column::QUANTITY:
        style.get(Any()).set(TextAlign(Qt::Alignment(Qt::AlignRight)));
        //style.get(Any() > is_a<TextBox>()).set(TextAlign(Qt::AlignRight));
      default:
        style.get(Any()).
          set(horizontal_padding(scale_width(8)));
        break;
    }
  });
  auto cell = [&] () -> QWidget* {
    if(static_cast<Column>(column) == Column::REGION) {
      return new CustomPopupBox(*editor);
    }
    return editor;
  }();
  return cell;
}

EditableBox* OrderTasksKeyBindingsForm::make_editor(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  auto column_id = static_cast<Column>(column);
  auto input_box = [&] () -> AnyInputBox* {
    switch(column_id) {
      case Column::NAME:
        return new AnyInputBox(*new TextBox(make_custom_list_value_model(
          std::make_shared<CustomColumnViewListModel<QString>>(table, column),
          row)));
      case Column::REGION:
        return new AnyInputBox(*new RegionBox(m_region_query_model,
          make_custom_list_value_model(
            std::make_shared<CustomColumnViewListModel<Region>>(table, column),
            row)));
      case Column::DESTINATION:
        {
          auto region_model = make_custom_list_value_model(
            std::make_shared<CustomColumnViewListModel<Region>>(
              table, static_cast<int>(Column::REGION)), row);
          auto query_model = std::make_shared<DestinationQueryModel>(
            std::move(region_model), m_destinations, m_markets);
          auto current_model = std::make_shared<DestinationValueModel>(
            make_custom_list_value_model(
              std::make_shared<CustomColumnViewListModel<Destination>>(table,
                column), row), query_model);
          return new AnyInputBox(*new DestinationBox(std::move(query_model),
            std::move(current_model)));
        }
      case Column::ORDER_TYPE:
        return new AnyInputBox(*make_order_type_box(
          make_custom_list_value_model(
            std::make_shared<CustomColumnViewListModel<OrderType>>(table,
              column), row)));
      case Column::SIDE:
        return new AnyInputBox(*make_side_box(make_custom_list_value_model(
          std::make_shared<CustomColumnViewListModel<Side>>(table, column),
          row)));
      case Column::QUANTITY:
        {
          auto model =
            std::make_shared<ScalarValueModelDecorator<optional<Quantity>>>(
              make_custom_list_value_model(
                std::make_shared<CustomColumnViewListModel<optional<Quantity>>>(
                  table, column), row));
          return new AnyInputBox(*new QuantityBox(std::move(model),
            make_quantity_modifiers()));
        }
      case Column::TIME_IN_FORCE:
        return new AnyInputBox(*make_time_in_force_box(
          make_custom_list_value_model(
            std::make_shared<CustomColumnViewListModel<TimeInForce>>(table,
              column), row)));
      case Column::TAG:
        return new AnyInputBox(*make_label(""));
      case Column::KEY:
        return new AnyInputBox(*new KeyInputBox(
          make_validated_value_model<QKeySequence>(&test_key_sequence,
            make_custom_list_value_model(
              std::make_shared<CustomColumnViewListModel<QKeySequence>>(
                table, column), row))));
      default:
        return nullptr;
    }
  }();
  if(!input_box) {
    return nullptr;
  }
  if(column_id == Column::REGION || column_id == Column::KEY) {
    input_box->connect_submit_signal([=] (const auto& region) {
      m_unique_key_bindings->update();
    });
  }
  return new EditableBox(*input_box);;
}

EditableBox* OrderTasksKeyBindingsForm::make_empty_editor(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  auto input_box = [&] () -> AnyInputBox* {
    switch(static_cast<Column>(column)) {
      case Column::NAME:
        return new AnyInputBox(*new TextBox(""));
      case Column::REGION:
        return new AnyInputBox(*new RegionBox(m_region_query_model));
      case Column::DESTINATION:
        return new AnyInputBox(*new DestinationBox(
          std::make_shared<LocalComboBoxQueryModel>()));
      case Column::ORDER_TYPE:
        return new AnyInputBox(*make_order_type_box(OrderType::NONE));
      case Column::SIDE:
        return new AnyInputBox(*make_side_box(Side::NONE));
      case Column::QUANTITY:
        return new AnyInputBox(*new QuantityBox(make_quantity_modifiers()));
      case Column::TIME_IN_FORCE:
        return new AnyInputBox(*make_time_in_force_box(TimeInForce()));
      case Column::TAG:
        return new AnyInputBox(*make_label(""));
      case Column::KEY:
        return new AnyInputBox(*(new KeyInputBox(
          make_validated_value_model<QKeySequence>(&test_key_sequence,
            std::make_shared<LocalValueModel<QKeySequence>>()))));
      default:
        return nullptr;
    }
  }();
  if(!input_box) {
    return nullptr;
  }
  input_box->connect_submit_signal(
    std::bind_front(&OrderTasksKeyBindingsForm::on_submit, this, input_box,
      static_cast<Column>(column)));
  return new EditableBox(*input_box);;
}

void OrderTasksKeyBindingsForm::on_submit(AnyInputBox* input_box, Column column,
    const AnyRef& submission) {
  auto argument = OrderTaskArguments();
  auto has_value = false;
  if(column == Column::NAME) {
    argument.m_name = any_cast<QString>(submission);
    if(!argument.m_name.isEmpty()) {
      has_value = true;
      input_box->get_current()->set(QString());
    }
  } else if(column == Column::REGION) {
    argument.m_region = any_cast<Region>(submission);
    if(argument.m_region != Region()) {
      has_value = true;
      input_box->get_current()->set(Region());
    }
  } else if(column == Column::DESTINATION) {
    argument.m_destination = any_cast<Destination>(submission);
    if(argument.m_destination != Destination()) {
      has_value = true;
      input_box->get_current()->set(Destination());
    }
  } else if(column == Column::ORDER_TYPE) {
    argument.m_order_type = any_cast<OrderType>(submission);
    if(argument.m_order_type != OrderType::NONE) {
      has_value = true;
      input_box->get_current()->set(OrderType());
    }
  } else if(column == Column::SIDE) {
    argument.m_side = any_cast<Side>(submission);
    if(argument.m_side != Side::NONE) {
      has_value = true;
      input_box->get_current()->set(Side());
    }
  } else if(column == Column::QUANTITY) {
    argument.m_quantity = any_cast<optional<Quantity>>(submission);
    if(argument.m_quantity) {
      has_value = true;
      input_box->get_current()->set(optional<Quantity>());
    }
  } else if(column == Column::TIME_IN_FORCE) {
    argument.m_time_in_force = any_cast<TimeInForce>(submission);
    if(argument.m_time_in_force != TimeInForce()) {
      has_value = true;
      input_box->get_current()->set(TimeInForce());
    }
  } else if(column == Column::KEY) {
    argument.m_key = any_cast<QKeySequence>(submission);
    if(!argument.m_key.isEmpty()) {
      has_value = true;
      input_box->get_current()->set(QKeySequence());
    }
  }
  if(has_value) {
    m_order_task_arguments->push(argument);
  }
}

void OrderTasksKeyBindingsForm::on_delete(int row) {
  m_order_task_arguments->remove(row);
}

void OrderTasksKeyBindingsForm::on_search(const QString& query) {
  m_table_view->set_filter([=] (const TableModel& model, int row) {
    if(query.isEmpty()) {
      return false;
    }
    return !m_table_match_cache->matches(row, query);
  });
}
