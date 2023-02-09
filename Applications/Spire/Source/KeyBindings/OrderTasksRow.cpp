#include "Spire/KeyBindings/OrderTasksRow.hpp"
#include <QMouseEvent>
#include "Spire/KeyBindings/GrabHandle.hpp"
#include "Spire/Spire/ColumnViewListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Spire/ValidatedValueModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DestinationBox.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/KeyInputBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/OrderTypeBox.hpp"
#include "Spire/Ui/PopupBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/RegionBox.hpp"
#include "Spire/Ui/SideBox.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TimeInForceBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  QWidget* find_tip_window(QWidget& parent) {
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
            m_local_query_model->add(displayText(destination.m_id).toLower(),
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
          !m_query_model->parse(displayText(get())).has_value()) {
        set(Destination());
      }
    }
  };

  class CustomPopupBox : public QWidget {
    public:
      explicit CustomPopupBox(QWidget& body, QWidget* parent = nullptr)
          : QWidget(parent) {
        m_popup_box = new PopupBox(body);
        enclose(*this, *m_popup_box);
        proxy_style(*this, *m_popup_box);
        setFocusPolicy(Qt::ClickFocus);
        m_popup_box->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_tip_window = find_tip_window(body);
      }

    protected:
      bool event(QEvent* event) override {
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

      void keyPressEvent(QKeyEvent* event) override {
        switch(event->key()) {
          case Qt::Key_Enter:
          case Qt::Key_Return:
          case Qt::Key_Backspace:
            QCoreApplication::sendEvent(&m_popup_box->get_body(), event);
            break;
          default:
            auto text = event->text();
            if(is_a_word(text)) {
              QCoreApplication::sendEvent(&m_popup_box->get_body(), event);
            } else {
              QWidget::keyPressEvent(event);
            }
        }
      }

    private:
      PopupBox* m_popup_box;
      QWidget* m_tip_window;
  };

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
}

OrderTasksRow::OrderTasksRow(std::shared_ptr<ListModel<OrderTask>> order_tasks,
  int row)
  : m_order_tasks(std::move(order_tasks)),
    m_row_index(row),
    m_is_draggable(true),
    m_is_ignore_filters(false),
    m_is_out_of_range(false),
    m_operation_connection(m_order_tasks->connect_operation_signal(
      std::bind_front(&OrderTasksRow::on_operation, this))) {}

int OrderTasksRow::get_row_index() const {
  return m_row_index;
}

QWidget* OrderTasksRow::get_row() const {
  return m_row;
}

bool OrderTasksRow::is_draggable() const {
  return m_is_draggable;
}

void OrderTasksRow::set_draggable(bool is_draggable) {
  m_is_draggable = is_draggable;
}

bool OrderTasksRow::is_ignore_filters() const {
  return m_is_ignore_filters;
}

void OrderTasksRow::set_ignore_filters(bool is_ignore_filters) {
  m_is_ignore_filters = is_ignore_filters;
}

bool OrderTasksRow::is_out_of_range() const {
  return m_is_out_of_range;
}

void OrderTasksRow::set_out_of_range(bool is_out_of_range) {
  if(is_out_of_range == m_is_out_of_range) {
    return;
  }
  m_is_out_of_range = is_out_of_range;
  if(m_row) {
    if(m_is_out_of_range) {
      match(*m_row, OutOfRangeRow());
    } else {
      unmatch(*m_row, OutOfRangeRow());
    }
  }
}

OrderTasksRow::TableCell OrderTasksRow::build_cell(
    const std::shared_ptr<ComboBox::QueryModel>& region_query_model,
    const DestinationDatabase& destinations,
    const MarketDatabase& markets,
    const std::shared_ptr<TableModel>& table, int row, int column) {
  if(row == table->get_row_size() - 1) {
    m_is_draggable = false;
    m_is_ignore_filters = true;
  }
  auto column_id = static_cast<Column>(column);
  if(column_id == Column::GRAB_HANDLE) {
    m_grab_handle = new GrabHandle();
    update_style(*m_grab_handle, [] (auto& style) {
      style.get(Any() > is_a<Icon>()).set(Visibility::NONE);
    });
    return {m_grab_handle, nullptr};
  }
  if(m_grab_handle && !m_row) {
    m_grab_handle->parentWidget()->setFocusPolicy(Qt::NoFocus);
    m_row = m_grab_handle->parentWidget()->parentWidget()->parentWidget();
    if(is_out_of_range()) {
      match(*m_row, OutOfRangeRow());
    }
    if(is_draggable()) {
      make_hover_observer();
    }
    update_style(*m_row, [] (auto& style) {
      style.get(Any() > HoveredGrabHandle() > is_a<Icon>()).
        set(Visibility::VISIBLE);
      style.get(Hover()).
        set(BackgroundColor(0xF2F2FF));
    });
  }
  auto editor = [&] {
    if(row == table->get_row_size() - 1) {
      return make_empty_editor(region_query_model, destinations, markets, table,
        row, column);
    }
    return make_editor(region_query_model, destinations, markets, table, row,
      column);
  }();
  update_style(*editor, [&] (auto& style) {
    switch(column_id) {
      case Column::ORDER_TYPE:
      case Column::SIDE:
      case Column::TIME_IN_FORCE:
        style.get(ReadOnly() > is_a<TextBox>()).
          set(horizontal_padding(scale_width(8)));
        break;
      case Column::QUANTITY:
        style.get(Any() > is_a<TextBox>()).set(TextAlign(Qt::AlignRight));
      default:
        style.get(ReadOnly()).
          set(horizontal_padding(scale_width(8)));
        break;
    }
  });
  auto cell = [&] () -> QWidget* {
    if(column_id == Column::REGION || column_id == Column::QUANTITY) {
      return new CustomPopupBox(*editor);
    }
    return editor;
  }();
  if(column_id == Column::QUANTITY) {
    cell->layout()->itemAt(0)->widget()->setSizePolicy(
      QSizePolicy::Expanding, QSizePolicy::Expanding);
  }
  editor->connect_start_edit_signal([=] {
    match(*cell, Editing());
  });
  editor->connect_end_edit_signal([=] {
    unmatch(*cell, Editing());
  });
  return {cell, editor};
}

void OrderTasksRow::make_hover_observer() {
  m_hover_observer = std::make_unique<HoverObserver>(*m_row);
  m_hover_observer->connect_state_signal([=] (auto state) {
    if(!m_grab_handle) {
      return;
    }
    if(state == HoverObserver::State::NONE) {
      unmatch(*m_grab_handle, HoveredGrabHandle());
    } else {
      match(*m_grab_handle, HoveredGrabHandle());
    }
  });
}

EditableBox* OrderTasksRow::make_editor(
    const std::shared_ptr<ComboBox::QueryModel>& region_query_model,
    const DestinationDatabase& destinations, const MarketDatabase& markets,
    const std::shared_ptr<TableModel>& table, int row, int column) {
  auto input_box = [&] () -> AnyInputBox* {
    switch(static_cast<Column>(column)) {
      case Column::NAME:
        return new AnyInputBox(*new TextBox(make_custom_list_value_model(
          std::make_shared<CustomColumnViewListModel<QString>>(table, column),
            row)));
      case Column::REGION:
        return new AnyInputBox(*new RegionBox(region_query_model,
          make_custom_list_value_model(
            std::make_shared<CustomColumnViewListModel<Region>>(table, column),
              row)));
      case Column::DESTINATION:
        {
          auto region_model = make_custom_list_value_model(
            std::make_shared<CustomColumnViewListModel<Region>>(
              table, static_cast<int>(Column::REGION)), row);
          auto query_model = std::make_shared<DestinationQueryModel>(
            std::move(region_model), destinations, markets);
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
    return new EditableBox(*input_box);;
}

EditableBox* OrderTasksRow::make_empty_editor(
    const std::shared_ptr<ComboBox::QueryModel>& region_query_model,
    const DestinationDatabase& destinations, const MarketDatabase& markets,
    const std::shared_ptr<TableModel>& table, int row, int column) {
  auto input_box = [&] () -> AnyInputBox* {
    switch(static_cast<Column>(column)) {
      case Column::NAME:
        return new AnyInputBox(*new TextBox(""));
      case Column::REGION:
        return new AnyInputBox(*new RegionBox(region_query_model));
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
    std::bind_front(&OrderTasksRow::on_submit, this, input_box,
      static_cast<Column>(column)));
  return new EditableBox(*input_box);;
}

void OrderTasksRow::on_operation(
    const ListModel<OrderTask>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<OrderTask>::AddOperation& operation) {
      if(m_row_index >= operation.m_index) {
        ++m_row_index;
      }
    },
    [&] (const ListModel<OrderTask>::RemoveOperation& operation) {
      if(m_row_index == operation.m_index) {
        m_row_index = -1;
      } else if(m_row_index > operation.m_index) {
        --m_row_index;
      }
    });
}

void OrderTasksRow::on_submit(AnyInputBox* input_box, Column column,
    const AnyRef& submission) {
  auto order_task = OrderTask();
  auto has_value = false;
  if(column == Column::NAME) {
    order_task.m_name = any_cast<QString>(submission);
    if(!order_task.m_name.isEmpty()) {
      has_value = true;
      input_box->get_current()->set(QString());
    }
  } else if(column == Column::REGION) {
    order_task.m_region = any_cast<Region>(submission);
    if(order_task.m_region != Region()) {
      has_value = true;
      input_box->get_current()->set(Region());
    }
  } else if(column == Column::DESTINATION) {
    order_task.m_destination = any_cast<Destination>(submission);
    if(order_task.m_destination != Destination()) {
      has_value = true;
      input_box->get_current()->set(Destination());
    }
  } else if(column == Column::ORDER_TYPE) {
    order_task.m_order_type = any_cast<OrderType>(submission);
    if(order_task.m_order_type != OrderType::NONE) {
      has_value = true;
      input_box->get_current()->set(OrderType());
    }
  } else if(column == Column::SIDE) {
    order_task.m_side = any_cast<Side>(submission);
    if(order_task.m_side != Side::NONE) {
      has_value = true;
      input_box->get_current()->set(Side());
    }
  } else if(column == Column::QUANTITY) {
    order_task.m_quantity = any_cast<optional<Quantity>>(submission);
    if(order_task.m_quantity) {
      has_value = true;
      input_box->get_current()->set(optional<Quantity>());
    }
  } else if(column == Column::TIME_IN_FORCE) {
    order_task.m_time_in_force = any_cast<TimeInForce>(submission);
    if(order_task.m_time_in_force != TimeInForce()) {
      has_value = true;
      input_box->get_current()->set(TimeInForce());
    }
  } else if(column == Column::KEY) {
    order_task.m_key = any_cast<QKeySequence>(submission);
    if(!order_task.m_key.isEmpty()) {
      has_value = true;
      input_box->get_current()->set(QKeySequence());
    }
  }
  if(has_value) {
    m_order_tasks->push(order_task);
  }
}
