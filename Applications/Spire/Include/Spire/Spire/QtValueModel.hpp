#ifndef SPIRE_QT_VALUE_MODEL_HPP
#define SPIRE_QT_VALUE_MODEL_HPP
#include <condition_variable>
#include <mutex>
#include <QCoreApplication>
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

namespace Spire {

  /**
   * Implements a thread-safe ValueModel that ensures updates are signalled to
   * Qt's main event loop thread.
   */
  template<typename T>
  class QtValueModel : public ValueModel<T> {
    public:
      using Type = typename ValueModel<T>::Type;
      using UpdateSignal = typename ValueModel<T>::UpdateSignal;

      /** Constructs a default model. */
      QtValueModel() = default;

      /**
       * Constructs a model with an initial value.
       * @param value The initial value.
       */
      explicit QtValueModel(Type value);

      const Type& get() const;
      QValidator::State test(const Type& value) const override;
      QValidator::State set(const Type& value) override;
      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const;

    private:
      mutable std::recursive_mutex m_mutex;
      mutable std::condition_variable_any m_update_condition;
      LocalValueModel<Type> m_current;
      EventHandler m_event_handler;
  };

  template<typename T>
  QtValueModel<T>::QtValueModel(Type value)
    : m_current(std::move(value)) {}

  template<typename T>
  const typename QtValueModel<T>::Type& QtValueModel<T>::get() const {
    auto lock = std::lock_guard(m_mutex);
    return m_current.get();
  }

  template<typename T>
  QValidator::State QtValueModel<T>::test(const Type& value) const {
    return QValidator::State::Acceptable;
  }

  template<typename T>
  QValidator::State QtValueModel<T>::set(const Type& value) {
    if(QCoreApplication::instance() &&
        QThread::currentThread() == QCoreApplication::instance()->thread()) {
      auto lock = std::lock_guard(m_mutex);
      return m_current.set(value);
    }
    auto lock = std::unique_lock(m_mutex);
    auto is_updated = false;
    auto state = QValidator::State();
    m_event_handler.push([&] {
      auto lock = std::lock_guard(m_mutex);
      state = m_current.set(value);
      is_updated = true;
      m_update_condition.notify_all();
    });
    while(!is_updated) {
      m_update_condition.wait(lock);
    }
    return state;
  }

  template<typename T>
  boost::signals2::connection QtValueModel<T>::connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const {
    auto lock = std::lock_guard(m_mutex);
    return m_current.connect_update_signal(slot);
  }
}

#endif
