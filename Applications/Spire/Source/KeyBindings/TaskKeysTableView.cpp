#include "Spire/KeyBindings/TaskKeysTableView.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/KeyBindings/AdditionalTagsBox.hpp"
#include "Spire/KeyBindings/OrderTaskArgumentsTableModel.hpp"
#include "Spire/KeyBindings/QuantitySettingBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ProxyValueModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"
#include "Spire/Spire/TableValueModel.hpp"
#include "Spire/Spire/ValidatedValueModel.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/BreakoutBox.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/DestinationBox.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/EditableTableView.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/EmptyTableFilter.hpp"
#include "Spire/Ui/KeyInputBox.hpp"
#include "Spire/Ui/OrderTypeBox.hpp"
#include "Spire/Ui/RecycledTableViewItemBuilder.hpp"
#include "Spire/Ui/RegionBox.hpp"
#include "Spire/Ui/SideBox.hpp"
#include "Spire/Ui/SingleSelectionModel.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TimeInForceBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  bool comparator(const AnyRef& left, int left_row, const AnyRef& right,
      int right_row, int column) {
    if(left.get_type() == typeid(QuantitySetting) &&
        right.get_type() == typeid(QuantitySetting)) {
      return to_text(any_cast<QuantitySetting>(left)) <
        to_text(any_cast<QuantitySetting>(right));
    }
    return compare(left, right);
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
    auto push = [&] (const QString& name, const QString& short_name,
        TableHeaderItem::Order order = TableHeaderItem::Order::NONE) {
      model->push({name, short_name, order, TableFilter::Filter::UNFILTERED});
    };
    push(QObject::tr("Name"), QObject::tr("Name"));
    push(QObject::tr("Region"), QObject::tr("Region"));
    push(QObject::tr("Destination"), QObject::tr("Dest"));
    push(QObject::tr("Order Type"), QObject::tr("Ord Type"));
    push(QObject::tr("Side"), QObject::tr("Side"));
    push(QObject::tr("Quantity"), QObject::tr("Qty"));
    push(QObject::tr("Time in Force"), QObject::tr("TIF"));
    push(QObject::tr("Tags"), QObject::tr("Tags"),
      TableHeaderItem::Order::UNORDERED);
    push(QObject::tr("Key"), QObject::tr("Key"));
    return model;
  }

  auto make_header_widths() {
    auto widths = std::vector<int>();
    widths.push_back(scale_width(160));
    widths.push_back(scale_width(154));
    widths.push_back(scale_width(64));
    widths.push_back(scale_width(78));
    widths.push_back(scale_width(56));
    widths.push_back(scale_width(82));
    widths.push_back(scale_width(55));
    widths.push_back(scale_width(80));
    widths.push_back(scale_width(130));
    return widths;
  }

  struct RegionKeyHash {
    std::size_t operator()(
        const std::pair<Region, QKeySequence>& region_key) const {
      auto seed = std::size_t(0);
      hash_combine(seed, region_key.first);
      hash_combine(seed, qHash(region_key.second));
      return seed;
    }
  };

  struct UniqueTaskKeyTableModel : TableModel {
    static const auto KEY_INDEX = static_cast<int>(OrderTaskColumns::KEY);
    static const auto REGION_INDEX = static_cast<int>(OrderTaskColumns::REGION);
    std::shared_ptr<TableModel> m_source;
    TableModelTransactionLog m_transaction;
    std::unordered_set<std::pair<Region, QKeySequence>, RegionKeyHash>
      m_region_keys;
    scoped_connection m_source_connection;

    explicit UniqueTaskKeyTableModel(std::shared_ptr<TableModel> source)
        : m_source(std::move(source)),
          m_source_connection(m_source->connect_operation_signal(
            std::bind_front(&UniqueTaskKeyTableModel::on_operation, this))) {
      for(auto row = 0; row < get_row_size(); ++row) {
        if(auto& key = get<QKeySequence>(row, KEY_INDEX); !key.isEmpty()) {
          m_region_keys.insert({get<Region>(row, REGION_INDEX), key});
        }
      }
    }

    int get_row_size() const override {
      return m_source->get_row_size();
    }

    int get_column_size() const override {
      return m_source->get_column_size();
    }

    AnyRef at(int row, int column) const override {
      return m_source->at(row, column);
    }

    QValidator::State set(int row, int column, const std::any& value) override {
      auto find_conflicting_row =
        [&] (const Region& region, const QKeySequence& key) {
          if(key.isEmpty() || m_region_keys.insert({region, key}).second) {
            return -1;
          }
          for(auto i = 0; i < get_row_size(); ++i) {
            if(i != row && get<Region>(i, REGION_INDEX) == region &&
                get<QKeySequence>(i, KEY_INDEX) == key) {
              return i;
            }
          }
          return -1;
        };
      auto result = QValidator::State::Acceptable;
      m_transaction.transact([&] {
        if(column == REGION_INDEX) {
          auto& key = get<QKeySequence>(row, KEY_INDEX);
          m_region_keys.erase({get<Region>(row, REGION_INDEX), key});
          auto row =
            find_conflicting_row(std::any_cast<const Region&>(value), key);
          if(row != -1) {
            result = m_source->set(row, KEY_INDEX, QKeySequence());
          }
        } else if(column == KEY_INDEX) {
          auto& region = get<Region>(row, REGION_INDEX);
          m_region_keys.erase({region, get<QKeySequence>(row, KEY_INDEX)});
          auto row = find_conflicting_row(
            region, std::any_cast<const QKeySequence&>(value));
          if(row != -1) {
            result = m_source->set(row, KEY_INDEX, QKeySequence());
          }
        }
        if(result != QValidator::Invalid) {
          result = m_source->set(row, column, value);
        }
      });
      return result;
    }

    QValidator::State remove(int row) override {
      m_region_keys.erase(
        {get<Region>(row, REGION_INDEX), get<QKeySequence>(row, KEY_INDEX)});
      return m_source->remove(row);
    }

    connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override {
      return m_transaction.connect_operation_signal(slot);
    }

    void on_operation(const Operation& operation) {
      m_transaction.push(operation);
    }
  };

  struct ItemState {
    virtual ~ItemState() = default;
    std::shared_ptr<void> m_proxy;

    ItemState(std::shared_ptr<void> proxy)
      : m_proxy(std::move(proxy)) {}
  };

  struct DestinationState : ItemState {
    std::shared_ptr<ProxyValueModel<Region>> m_region;

    DestinationState(std::shared_ptr<void> proxy,
      std::shared_ptr<ProxyValueModel<Region>> region)
      : ItemState(std::move(proxy)),
        m_region(std::move(region)) {}
  };

  struct AdditionalTagsState : ItemState {
    std::shared_ptr<ProxyValueModel<Destination>> m_destination;
    std::shared_ptr<ProxyValueModel<Region>> m_region;

    AdditionalTagsState(std::shared_ptr<void> proxy,
      std::shared_ptr<ProxyValueModel<Destination>> destination,
      std::shared_ptr<ProxyValueModel<Region>> region)
      : ItemState(std::move(proxy)),
        m_destination(std::move(destination)),
        m_region(std::move(region)) {}
  };

  class BreakoutInputBox : public QWidget {
    public:
      explicit BreakoutInputBox(AnyInputBox& input_box,
          QWidget* parent = nullptr)
          : QWidget(parent),
            m_input_box(&input_box),
            m_is_setting_read_only(false) {
        m_breakout_box = new BreakoutBox(*m_input_box, this);
        enclose(*this, *m_breakout_box);
        setFocusProxy(m_breakout_box);
        proxy_style(*this, *m_breakout_box);
      }

      const std::shared_ptr<AnyValueModel>& get_current() const {
        return m_input_box->get_current();
      }

      const AnyRef& get_submission() const {
        return m_input_box->get_submission();
      }

      void set_placeholder(const QString& placeholder) {
        m_input_box->set_placeholder(placeholder);
      }

      bool is_read_only() const {
        return m_input_box->is_read_only();
      }

      void set_read_only(bool read_only) {
        if(m_is_setting_read_only) {
          return;
        }
        m_is_setting_read_only = true;
        if(read_only == is_read_only()) {
          return;
        }
        m_input_box->set_read_only(read_only);
        if(read_only) {
          m_input_box->setFixedHeight(
            m_input_box->minimumSizeHint().height());
          restore();
        } else {
          m_input_box->setMinimumHeight(0);
          m_input_box->setMaximumHeight(QWIDGETSIZE_MAX);
          breakout();
        }
        m_is_setting_read_only = false;
      }

      connection connect_submit_signal(
          const AnyInputBox::SubmitSignal::slot_type& slot) const {
        return m_input_box->connect_submit_signal(slot);
      }

      connection connect_reject_signal(
          const AnyInputBox::RejectSignal::slot_type& slot) const {
        return m_input_box->connect_reject_signal(slot);
      }

    protected:
      bool eventFilter(QObject* watched, QEvent* event) {
        if(watched == m_input_box && event->type() == QEvent::KeyPress) {
          auto& key_event = *static_cast<QKeyEvent*>(event);
          if(key_event.key() == Qt::Key_Escape) {
            if(auto parent = parentWidget()) {
              QCoreApplication::sendEvent(parent, event);
            }
          }
        } else if(watched == m_breakout_box &&
            m_breakout_box->is_broken_out()) {
          if(event->type() == QEvent::FocusIn) {
            m_input_box->setFocus();
          } else if(event->type() == QEvent::KeyPress) {
            if(auto focus_proxy = find_focus_proxy(*m_input_box)) {
              QCoreApplication::sendEvent(focus_proxy, event);
              return true;
            }
          }
        }
        return QWidget::eventFilter(watched, event);
      }

    private:
      BreakoutBox* m_breakout_box;
      AnyInputBox* m_input_box;
      optional<FocusObserver> m_input_box_focus_observer;
      bool m_is_setting_read_only;

      void breakout() {
        m_breakout_box->breakout();
        match(*this, PopUp());
        m_input_box->installEventFilter(this);
        m_breakout_box->installEventFilter(this);
        m_input_box_focus_observer.emplace(*m_input_box);
        m_input_box_focus_observer->connect_state_signal(
          std::bind_front(&BreakoutInputBox::on_input_box_focus, this));
      }

      void restore() {
        m_breakout_box->restore();
        unmatch(*this, PopUp());
        m_input_box_focus_observer = none;
        m_input_box->removeEventFilter(this);
        m_breakout_box->removeEventFilter(this);
      }

      void on_input_box_focus(FocusObserver::State state) {
        if(state == FocusObserver::State::NONE &&
            m_breakout_box->is_broken_out()) {
          if(auto parent = parentWidget()) {
            auto key_event =
              QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
            QCoreApplication::sendEvent(parent, &key_event);
          }
        }
      }
  };

  struct TaskKeysTableViewItemBuilder {
    std::shared_ptr<RegionQueryModel> m_regions;
    DestinationDatabase m_destinations;
    MarketDatabase m_markets;
    AdditionalTagDatabase m_additional_tags;
    std::map<QWidget*, std::shared_ptr<ItemState>> m_item_states;

    EditableBox* mount(
        const std::shared_ptr<TableModel>& table, int row, int column) {
      auto make_proxy = [&] <typename T> () {
        return make_proxy_value_model(
          make_table_value_model<T>(table, row, column));
      };
      auto column_id = static_cast<OrderTaskColumns>(column);
      auto [input_box, proxy] =
        [&] () -> std::tuple<EditableBox*, std::shared_ptr<ItemState>> {
          if(column_id == OrderTaskColumns::NAME) {
            auto current = make_proxy.operator ()<QString>();
            return {new EditableBox(
              *new TextBox(current)), std::make_shared<ItemState>(current)};
          } else if(column_id == OrderTaskColumns::REGION) {
            auto current = make_proxy.operator ()<Region>();
            auto region_box = new RegionBox(m_regions, current);
            auto breakout_input_box =
              new BreakoutInputBox(*new AnyInputBox(*region_box));
            auto editable_box =
              new EditableBox(*new AnyInputBox(*breakout_input_box), [] {
                return Region();
              });
            return {editable_box, std::make_shared<ItemState>(current)};
          } else if(column_id == OrderTaskColumns::DESTINATION) {
            auto region = make_proxy_value_model(
              make_table_value_model<Region>(table, row,
                static_cast<int>(OrderTaskColumns::REGION)));
            auto destinations = make_region_filtered_destination_list(
              m_destinations, m_markets, region);
            auto current = make_proxy.operator ()<Destination>();
            return {new EditableBox(
              *make_destination_box(current, std::move(destinations))),
              std::make_shared<DestinationState>(current, region)};
          } else if(column_id == OrderTaskColumns::ORDER_TYPE) {
            auto current = make_proxy.operator ()<OrderType>();
            return {new EditableBox(*make_order_type_box(current)),
              std::make_shared<ItemState>(current)};
          } else if(column_id == OrderTaskColumns::SIDE) {
            auto current = make_proxy.operator ()<Side>();
            return {new EditableBox(*make_side_box(current)),
              std::make_shared<ItemState>(current)};
          } else if(column_id == OrderTaskColumns::QUANTITY) {
            auto current = make_proxy.operator ()<QuantitySetting>();
            return {new EditableBox(*make_quantity_setting_box(current)),
              std::make_shared<ItemState>(current)};
          } else if(column_id == OrderTaskColumns::TIME_IN_FORCE) {
            auto current = make_proxy.operator ()<TimeInForce>();
            return {new EditableBox(*make_time_in_force_box(current)),
              std::make_shared<ItemState>(current)};
          } else if(column_id == OrderTaskColumns::TAGS) {
            auto destination = make_proxy_value_model(
              make_table_value_model<Destination>(
                table, row, static_cast<int>(OrderTaskColumns::DESTINATION)));
            auto region = make_proxy_value_model(make_table_value_model<Region>(
              table, row, static_cast<int>(OrderTaskColumns::REGION)));
            auto current = make_proxy.operator ()<std::vector<AdditionalTag>>();
            return {new EditableBox(*new AdditionalTagsBox(
              current, m_additional_tags, destination, region)),
              std::make_shared<AdditionalTagsState>(
                current, destination, region)};
          } else {
            auto proxy = make_proxy.operator ()<QKeySequence>();
            auto current =
              make_validated_value_model(&key_input_box_validator, proxy);
            return {new EditableBox(*new KeyInputBox(current),
              [] (const auto& key) {
                return key_input_box_validator(key) != QValidator::Invalid;
              }), std::make_shared<ItemState>(proxy)};
          }
        }();
      m_item_states[input_box] = std::move(proxy);
      return input_box;
    }

    void reset(QWidget& widget,
        const std::shared_ptr<TableModel>& table, int row, int column) {
      auto update_proxy = [&] <typename T> () {
        auto& state = *m_item_states[&widget];
        std::static_pointer_cast<ProxyValueModel<T>>(state.m_proxy)->set_source(
          make_table_value_model<T>(table, row, column));
      };
      auto column_id = static_cast<OrderTaskColumns>(column);
      if(column_id == OrderTaskColumns::NAME) {
        update_proxy.operator ()<QString>();
      } else if(column_id == OrderTaskColumns::REGION) {
        update_proxy.operator ()<Region>();
      } else if(column_id == OrderTaskColumns::DESTINATION) {
        auto& state = static_cast<DestinationState&>(*m_item_states[&widget]);
        auto proxy =
          std::static_pointer_cast<ProxyValueModel<Destination>>(state.m_proxy);
        auto temporary_model =
          std::make_shared<LocalDestinationModel>(proxy->get());
        proxy->set_source(temporary_model);
        state.m_region->set_source(make_table_value_model<Region>(
          table, row, static_cast<int>(OrderTaskColumns::REGION)));
        update_proxy.operator ()<Destination>();
      } else if(column_id == OrderTaskColumns::ORDER_TYPE) {
        update_proxy.operator ()<OrderType>();
      } else if(column_id == OrderTaskColumns::SIDE) {
        update_proxy.operator ()<Side>();
      } else if(column_id == OrderTaskColumns::QUANTITY) {
        update_proxy.operator ()<QuantitySetting>();
      } else if(column_id == OrderTaskColumns::TIME_IN_FORCE) {
        update_proxy.operator ()<TimeInForce>();
      } else if(column_id == OrderTaskColumns::TAGS) {
        auto& state =
          static_cast<AdditionalTagsState&>(*m_item_states[&widget]);
        auto proxy = std::static_pointer_cast<
          ProxyValueModel<std::vector<AdditionalTag>>>(state.m_proxy);
        auto temporary_model =
          std::make_shared<LocalValueModel<std::vector<AdditionalTag>>>(
            proxy->get());
        proxy->set_source(temporary_model);
        state.m_destination->set_source(make_table_value_model<Destination>(
          table, row, static_cast<int>(OrderTaskColumns::DESTINATION)));
        state.m_region->set_source(make_table_value_model<Region>(
          table, row, static_cast<int>(OrderTaskColumns::REGION)));
        update_proxy.operator ()<std::vector<AdditionalTag>>();
      } else if(column_id == OrderTaskColumns::KEY) {
        update_proxy.operator ()<QKeySequence>();
      }
    }

    void unmount(QWidget* widget) {
      delete widget;
    }
  };
}

TableView* Spire::make_task_keys_table_view(
    std::shared_ptr<OrderTaskArgumentsListModel> order_task_arguments,
    std::shared_ptr<RegionQueryModel> regions, DestinationDatabase destinations,
    MarketDatabase markets, AdditionalTagDatabase additional_tags,
    QWidget* parent) {
  auto table =
    make_order_task_arguments_table_model(std::move(order_task_arguments));
  auto builder = EditableTableViewBuilder(
    std::make_shared<UniqueTaskKeyTableModel>(std::move(table))).
    set_header(make_header_model()).
    set_item_builder(RecycledTableViewItemBuilder(TaskKeysTableViewItemBuilder(
      regions, destinations, markets, additional_tags))).
    set_comparator(&comparator);
  auto table_view = builder.make();
  auto widths = make_header_widths();
  for(auto i = 0; i < std::ssize(widths); ++i) {
    table_view->get_header().get_widths()->set(i + 1, widths[i]);
  }
  update_style(*table_view, [] (auto& style) {
    style.get(Any() > is_a<TableBody>() >
        Row() > is_a<TableItem>() > is_a<EditableBox>() > is_a<DecimalBox>()).
      set(TextAlign(Qt::Alignment(Qt::AlignRight)));
    style.get(Any() > is_a<TableBody>() > Selected() > is_a<TableItem>() >
        is_a<EditableBox>() > PopUp() << Selected()).
      set(BackgroundColor(QColor(0xE2E0FF)));
  });
  return table_view;
}
