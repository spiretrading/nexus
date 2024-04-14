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
        std::shared_ptr<ValueModel<Set>> set, typename Set::Type flag)
        : m_set(std::move(set)),
          m_flag(flag),
          m_value(m_set->get().Test(m_flag)),
          m_set_connection(m_set->connect_update_signal(
            std::bind_front(&EnumSetTestModel::on_update, this))) {}

      QValidator::State get_state() const override {
        return m_set->get_state();
      }

      const Type& get() const override {
        return m_value;
      }

      QValidator::State test(const Type& value) const override {
        auto set = m_set->get();
        if(value) {
          set.Set(m_flag);
        } else {
          set.Unset(m_flag);
        }
        return m_set->test(set);
      }

      QValidator::State set(const Type& value) override {
        auto set = m_set->get();
        if(value) {
          set.Set(m_flag);
        } else {
          set.Unset(m_flag);
        }
        return m_set->set(set);
      }

      boost::signals2::connection connect_update_signal(
          const typename UpdateSignal::slot_type& slot) const {
        return m_update_signal.connect(slot);
      }

    private:
      mutable UpdateSignal m_update_signal;
      std::shared_ptr<ValueModel<Set>> m_set;
      typename Set::Type m_flag;
      bool m_value;
      boost::signals2::scoped_connection m_set_connection;

      void on_update(const Set& value) {
        if(m_value == value.Test(m_flag)) {
          return;
        }
        m_value = !m_value;
        m_update_signal(bool(m_value));
      }
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
}

#endif
