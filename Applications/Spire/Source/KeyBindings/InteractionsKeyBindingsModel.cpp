#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <ranges>

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  template<typename T>
  class HierarchicalValueModel : public ValueModel<T> {
    public:
      using Type = typename ValueModel<T>::Type;
      using UpdateSignal = typename ValueModel<T>::UpdateSignal;
      using WriteSignal = Signal<void ()>;

      explicit HierarchicalValueModel(std::shared_ptr<ValueModel<Type>> parent)
        : m_parent(std::move(parent)),
BEAM_SUPPRESS_THIS_INITIALIZER()
          m_connection(m_parent->connect_update_signal(
            std::bind_front(&HierarchicalValueModel::on_update, this))),
BEAM_UNSUPPRESS_THIS_INITIALIZER()
          m_value(m_parent->get()) {}

      void detach() {
        m_connection.disconnect();
      }

      void reset() {
        m_connection = m_parent->connect_update_signal(
          std::bind_front(&HierarchicalValueModel::on_update, this));
        m_value.set(m_parent->get());
      }

      QValidator::State get_state() const override {
        if(m_connection.connected()) {
          return m_parent->get_state();
        }
        return m_value.get_state();
      }

      const Type& get() const {
        if(m_connection.connected()) {
          return m_parent->get();
        }
        return m_value.get();
      }

      QValidator::State test(const Type& value) const override {
        if(m_connection.connected()) {
          return m_parent->test(value);
        }
        return m_value.test(value);
      }

      QValidator::State set(const Type& value) override {
        if(m_connection.connected()) {
          reset_parent();
        }
        return m_value.set(value);
      }

      connection connect_write_signal(
          const WriteSignal::slot_type& slot) const {
        return m_write_signal.connect(slot);
      }

      connection connect_update_signal(
          const typename UpdateSignal::slot_type& slot) const {
        return m_value.connect_update_signal(slot);
      }

    private:
      mutable WriteSignal m_write_signal;
      std::shared_ptr<ValueModel<Type>> m_parent;
      LocalValueModel<Type> m_value;
      scoped_connection m_connection;

      void reset_parent() {
        detach();
        m_write_signal();
      }

      void on_update(const Type& value) {
        m_value.set(value);
      }
  };

  template<typename T>
  class HierarchicalScalarValueModel : public ScalarValueModel<T> {
    public:
      using Type = typename ScalarValueModel<T>::Type;
      using Scalar = typename ScalarValueModel<T>::Scalar;
      using UpdateSignal = typename ScalarValueModel<T>::UpdateSignal;
      using WriteSignal = Signal<void ()>;

      explicit HierarchicalScalarValueModel(
          std::shared_ptr<ScalarValueModel<Type>> parent)
        : m_parent(std::move(parent)),
BEAM_SUPPRESS_THIS_INITIALIZER()
          m_connection(m_parent->connect_update_signal(
            std::bind_front(&HierarchicalScalarValueModel::on_update, this))),
BEAM_UNSUPPRESS_THIS_INITIALIZER()
          m_value(m_parent->get()) {}

      void detach() {
        m_connection.disconnect();
      }

      void reset() {
        m_connection = m_parent->connect_update_signal(
          std::bind_front(&HierarchicalScalarValueModel::on_update, this));
        m_value.set(m_parent->get());
      }

      optional<Scalar> get_minimum() const override {
        if(m_connection.connected()) {
          return m_parent->get_minimum();
        }
        return m_value.get_minimum();
      }

      optional<Scalar> get_maximum() const override {
        if(m_connection.connected()) {
          return m_parent->get_maximum();
        }
        return m_value.get_maximum();
      }

      optional<Scalar> get_increment() const override {
        if(m_connection.connected()) {
          return m_parent->get_increment();
        }
        return m_value.get_increment();
      }

      QValidator::State get_state() const override {
        if(m_connection.connected()) {
          return m_parent->get_state();
        }
        return m_value.get_state();
      }

      const Type& get() const {
        if(m_connection.connected()) {
          return m_parent->get();
        }
        return m_value.get();
      }

      QValidator::State test(const Type& value) const override {
        if(m_connection.connected()) {
          return m_parent->test(value);
        }
        return m_value.test(value);
      }

      QValidator::State set(const Type& value) override {
        if(m_connection.connected()) {
          reset_parent();
        }
        return m_value.set(value);
      }

      connection connect_write_signal(
          const WriteSignal::slot_type& slot) const {
        return m_write_signal.connect(slot);
      }

      connection connect_update_signal(
          const typename UpdateSignal::slot_type& slot) const {
        return m_value.connect_update_signal(slot);
      }

    private:
      mutable WriteSignal m_write_signal;
      std::shared_ptr<ScalarValueModel<Type>> m_parent;
      LocalScalarValueModel<Type> m_value;
      scoped_connection m_connection;

      void reset_parent() {
        detach();
        m_write_signal();
      }

      void on_update(const Type& value) {
        m_value.set(value);
      }
  };

  using HierarchicalBooleanModel = HierarchicalValueModel<bool>;
  using HierarchicalQuantityModel = HierarchicalScalarValueModel<Quantity>;
  using HierarchicalMoneyModel = HierarchicalScalarValueModel<Money>;
}

InteractionsKeyBindingsModel::InteractionsKeyBindingsModel()
    : m_is_detached(true),
      m_default_quantity(std::make_shared<LocalQuantityModel>(100)),
      m_is_cancel_on_fill(std::make_shared<LocalBooleanModel>(false)) {
  for(auto& increment : m_quantity_increments) {
    increment = std::make_shared<LocalQuantityModel>(100);
  }
  for(auto& increment : m_price_increments) {
    increment = std::make_shared<LocalMoneyModel>(Money::CENT);
  }
}

InteractionsKeyBindingsModel::InteractionsKeyBindingsModel(
      std::shared_ptr<InteractionsKeyBindingsModel> parent)
    : m_is_detached(false) {
  auto default_quantity =
    std::make_shared<HierarchicalQuantityModel>(parent->get_default_quantity());
  m_connections.AddConnection(default_quantity->connect_write_signal(
    std::bind_front(&InteractionsKeyBindingsModel::on_write, this)));
  m_default_quantity = std::move(default_quantity);
  auto is_cancel_on_fill =
    std::make_shared<HierarchicalBooleanModel>(parent->is_cancel_on_fill());
  m_connections.AddConnection(is_cancel_on_fill->connect_write_signal(
    std::bind_front(&InteractionsKeyBindingsModel::on_write, this)));
  m_is_cancel_on_fill = std::move(is_cancel_on_fill);
  for(auto i : std::views::iota(0, MODIFIER_COUNT)) {
    auto model = std::make_shared<HierarchicalQuantityModel>(
      parent->get_quantity_increment(to_modifier(i)));
    m_connections.AddConnection(model->connect_write_signal(
      std::bind_front(&InteractionsKeyBindingsModel::on_write, this)));
    m_quantity_increments[i] = std::move(model);
  }
  for(auto i : std::views::iota(0, MODIFIER_COUNT)) {
    auto model = std::make_shared<HierarchicalMoneyModel>(
      parent->get_price_increment(to_modifier(i)));
    m_connections.AddConnection(model->connect_write_signal(
      std::bind_front(&InteractionsKeyBindingsModel::on_write, this)));
    m_price_increments[i] = std::move(model);
  }
}

bool InteractionsKeyBindingsModel::is_detached() const {
  return m_is_detached;
}

std::shared_ptr<QuantityModel>
    InteractionsKeyBindingsModel::get_default_quantity() const {
  return m_default_quantity;
}

std::shared_ptr<QuantityModel>
    InteractionsKeyBindingsModel::get_quantity_increment(
      Qt::KeyboardModifier modifier) const {
  return m_quantity_increments[to_index(modifier)];
}

std::shared_ptr<MoneyModel> InteractionsKeyBindingsModel::get_price_increment(
    Qt::KeyboardModifier modifier) const {
  return m_price_increments[to_index(modifier)];
}

std::shared_ptr<BooleanModel>
    InteractionsKeyBindingsModel::is_cancel_on_fill() const {
  return m_is_cancel_on_fill;
}

void InteractionsKeyBindingsModel::reset() {
  if(!m_is_detached) {
    return;
  }
  m_is_detached = false;
  std::static_pointer_cast<HierarchicalQuantityModel>(
    m_default_quantity)->reset();
  std::static_pointer_cast<HierarchicalBooleanModel>(
    m_is_cancel_on_fill)->reset();
  for(auto i : std::views::iota(0, MODIFIER_COUNT)) {
    std::static_pointer_cast<HierarchicalQuantityModel>(
      m_quantity_increments[i])->reset();
  }
  for(auto i : std::views::iota(0, MODIFIER_COUNT)) {
    std::static_pointer_cast<HierarchicalMoneyModel>(
      m_price_increments[i])->reset();
  }
}

void InteractionsKeyBindingsModel::on_write() {
  if(m_is_detached) {
    return;
  }
  m_is_detached = true;
  std::static_pointer_cast<HierarchicalQuantityModel>(
    m_default_quantity)->detach();
  std::static_pointer_cast<HierarchicalBooleanModel>(
    m_is_cancel_on_fill)->detach();
  for(auto i : std::views::iota(0, MODIFIER_COUNT)) {
    std::static_pointer_cast<HierarchicalQuantityModel>(
      m_quantity_increments[i])->detach();
  }
  for(auto i : std::views::iota(0, MODIFIER_COUNT)) {
    std::static_pointer_cast<HierarchicalMoneyModel>(
      m_price_increments[i])->detach();
  }
}

Qt::KeyboardModifier Spire::to_modifier(Qt::KeyboardModifiers modifiers) {
  if(modifiers.testFlag(Qt::ShiftModifier)) {
    return Qt::ShiftModifier;
  } else if(modifiers.testFlag(Qt::ControlModifier)) {
    return Qt::ControlModifier;
  } else if(modifiers.testFlag(Qt::AltModifier)) {
    return Qt::AltModifier;
  }
  return Qt::NoModifier;
}

Qt::KeyboardModifier Spire::to_modifier(int index) {
  if(index == 0) {
    return Qt::NoModifier;
  } else if(index == 1) {
    return Qt::ShiftModifier;
  } else if(index == 2) {
    return Qt::ControlModifier;
  } else if(index == 3) {
    return Qt::AltModifier;
  }
  throw std::out_of_range("Invalid keyboard modifier.");
}

int Spire::to_index(Qt::KeyboardModifier modifier) {
  if(modifier == Qt::NoModifier) {
    return 0;
  } else if(modifier == Qt::ShiftModifier) {
    return 1;
  } else if(modifier == Qt::ControlModifier) {
    return 2;
  } else if(modifier == Qt::AltModifier) {
    return 3;
  }
  throw std::out_of_range("Invalid keyboard modifier.");
}

Quantity Spire::get_default_order_quantity(
    const InteractionsKeyBindingsModel& interactions, const Security& security,
    Quantity position, Side side) {
  auto baseQuantity = interactions.get_default_quantity()->get();
  if(baseQuantity <= 0) {
    return 0;
  } else if(
      side == Side::BID && position < 0 || side == Side::ASK && position > 0) {
    return std::min(baseQuantity, Abs(position));
  }
  return baseQuantity - Abs(position) % baseQuantity;
}
