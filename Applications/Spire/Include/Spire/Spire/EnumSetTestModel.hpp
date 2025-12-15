#ifndef SPIRE_ENUM_SET_TEST_MODEL_HPP
#define SPIRE_ENUM_SET_TEST_MODEL_HPP
#include <functional>
#include <utility>
#include <Beam/Collections/EnumSet.hpp>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /**
   * Implements a ValueModel that tests and sets an Enum within an EnumSet.
   * @param <T> The type of EnumSet to test and set.
   */
  template<typename T>
  class EnumSetTestModel : public ValueModel<bool> {
    public:

      /** The type of EnumSet to test and set. */
      using Set = T;

      /**
       * Constructs an EnumSetTestModel.
       * @param set The EnumSet to test and set.
       * @param flag The specific enum within the <i>set</i> to inspect.
       */
      EnumSetTestModel(
        std::shared_ptr<ValueModel<Set>> set, typename Set::Type flag);

      QValidator::State get_state() const override;
      const Type& get() const override;
      QValidator::State test(const Type& value) const override;
      QValidator::State set(const Type& value) override;
      boost::signals2::connection connect_update_signal(
          const typename UpdateSignal::slot_type& slot) const override;

    private:
      mutable UpdateSignal m_update_signal;
      std::shared_ptr<ValueModel<Set>> m_set;
      typename Set::Type m_flag;
      bool m_value;
      boost::signals2::scoped_connection m_set_connection;

      void on_update(const Set& value);
  };

  /**
   * Makes an EnumSetTestModel.
   * @param set The EnumSet to test and set.
   * @param flag The specific enum within the <i>set</i> to inspect.
   */
  template<typename Set>
  auto make_enum_set_test_model(
      std::shared_ptr<Set> set, typename Set::Type::Type flag) {
    return std::make_shared<EnumSetTestModel<typename Set::Type>>(
      std::move(set), flag);
  }

  template<typename T>
  EnumSetTestModel<T>::EnumSetTestModel(
    std::shared_ptr<ValueModel<Set>> set, typename Set::Type flag)
    : m_set(std::move(set)),
      m_flag(flag),
      m_value(m_set->get().test(m_flag)),
      m_set_connection(m_set->connect_update_signal(
        std::bind_front(&EnumSetTestModel::on_update, this))) {}

  template<typename T>
  QValidator::State EnumSetTestModel<T>::get_state() const {
    return m_set->get_state();
  }

  template<typename T>
  const typename EnumSetTestModel<T>::Type& EnumSetTestModel<T>::get() const {
    return m_value;
  }

  template<typename T>
  QValidator::State EnumSetTestModel<T>::test(const Type& value) const {
    auto set = m_set->get();
    if(value) {
      set.set(m_flag);
    } else {
      set.reset(m_flag);
    }
    return m_set->test(set);
  }

  template<typename T>
  QValidator::State EnumSetTestModel<T>::set(const Type& value) {
    auto set = m_set->get();
    if(value) {
      set.set(m_flag);
    } else {
      set.reset(m_flag);
    }
    return m_set->set(set);
  }

  template<typename T>
  boost::signals2::connection EnumSetTestModel<T>::connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return m_update_signal.connect(slot);
  }

  template<typename T>
  void EnumSetTestModel<T>::on_update(const Set& value) {
    if(m_value == value.test(m_flag)) {
      return;
    }
    m_value = !m_value;
    m_update_signal(bool(m_value));
  }
}

#endif
