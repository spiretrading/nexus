#ifndef SPIRE_ANY_VALUE_MODEL_HPP
#define SPIRE_ANY_VALUE_MODEL_HPP
#include <any>
#include <memory>
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/AnyRef.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Implements a ValueModel by wrapping an existing ValueModel with a type-safe
   * generic interface.
   */
  class AnyValueModel : public ValueModel<AnyRef> {
    public:
      using Type = ValueModel<AnyRef>::Type;

      using UpdateSignal = ValueModel<AnyRef>::UpdateSignal;

      /**
       * Constructs an AnyValueModel over an existing ValueModel.
       * @param source The ValueModel to wrap.
       */
      template<typename T>
      explicit AnyValueModel(std::shared_ptr<ValueModel<T>> source);

      const Type& get() const override;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;

    private:
      mutable UpdateSignal m_update_signal;
      std::any m_source;
      LocalValueModel<Type> m_model;
      std::function<QValidator::State (const Type&)> m_test;
      boost::signals2::scoped_connection m_source_connection;
      boost::signals2::scoped_connection m_model_connection;

      template<typename T>
      const std::shared_ptr<ValueModel<T>>& get_source() const;
      template<typename T>
      QValidator::State on_test(const Type& value) const;
      template<typename T>
      void on_source_update(const T& value);
      template<typename T>
      void on_model_update(const Type& value);
  };

  template<typename T>
  AnyValueModel::AnyValueModel(std::shared_ptr<ValueModel<T>> source)
    : m_source(std::move(source)),
      m_model(const_cast<T&>(get_source<T>()->get())),
      m_test(std::bind_front(&AnyValueModel::on_test<T>, this)),
      m_source_connection(get_source<T>()->connect_update_signal(
        std::bind_front(&AnyValueModel::on_source_update<T>, this))),
      m_model_connection(m_model.connect_update_signal(
        std::bind_front(&AnyValueModel::on_model_update<T>, this))) {}

  template<typename T>
  const std::shared_ptr<ValueModel<T>>& AnyValueModel::get_source() const {
    return std::any_cast<const std::shared_ptr<ValueModel<T>>&>(m_source);
  }

  template<typename T>
  QValidator::State AnyValueModel::on_test(const Type& value) const {
    try {
      return get_source<T>()->test(any_cast<const T>(value));
    } catch(const std::bad_any_cast&) {
      return QValidator::State::Invalid;
    }
  }

  template<typename T>
  void AnyValueModel::on_source_update(const T& value) {
    auto blocker = boost::signals2::shared_connection_block(m_model_connection);
    m_model.set(const_cast<T&>(get_source<T>()->get()));
  }

  template<typename T>
  void AnyValueModel::on_model_update(const Type& value) {
    get_source<T>()->set(any_cast<const T>(value));
  }
}

#endif
