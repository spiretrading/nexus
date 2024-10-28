#ifndef SPIRE_VALUE_MODEL_HPP
#define SPIRE_VALUE_MODEL_HPP
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <boost/signals2/connection.hpp>
#include <QValidator>
#include "Spire/Spire/AnyRef.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Base class for all ValueModels. */
  class AnyValueModel {
    public:

      /**
       * Signals a change to the value.
       * @param value The updated value.
       */
      using UpdateSignal = Signal<void (AnyRef value)>;

      virtual ~AnyValueModel() = default;

      /**
       * Returns the state of the value, by default this is
       * <i>QValidator::State::Acceptable</i>
       */
      virtual QValidator::State get_state() const;

      /** Returns the value. */
      AnyRef get() const;

      /**
       * Tests if a value is valid, can be used to determine what a set
       * operation would return without actually modifying the model.
       * @param value The value to test.
       */
      QValidator::State test(AnyRef value) const;

      /**
       * Sets the value. By default this operation is a no-op that always
       * returns <i>QValidator::State::Invalid</i>.
       */
      QValidator::State set(AnyRef value);

      /** Connects a slot to the UpdateSignal. */
      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const;

    protected:

      /** Constructs an empty model. */
      AnyValueModel() = default;

      /** Returns a reference to the current value. */
      virtual AnyRef get_ref() const = 0;

      /** Performs a <i>test()</i> on a reference to a value. */
      virtual QValidator::State test_ref(AnyRef value) const = 0;

      /** Sets the value through a reference. */
      virtual QValidator::State set_ref(AnyRef value) = 0;

      /** Connects a slot to the UpdateSignal. */
      virtual boost::signals2::connection connect_update_signal_ref(
        const UpdateSignal::slot_type& slot) const = 0;

    private:
      AnyValueModel(const AnyValueModel&) = delete;
      AnyValueModel& operator =(const AnyValueModel&) = delete;
  };

  /**
   * Base class used to model a single value.
   * @param <T> The type of value being modeled.
   */
  template<typename T>
  class ValueModel : public AnyValueModel {
    public:

      /** The type of value being modeled. */
      using Type = T;

      /**
       * Signals a change to the value.
       * @param value The updated value.
       */
      using UpdateSignal = Signal<void (const Type& value)>;

      /** Returns the value. */
      virtual const Type& get() const = 0;

      /**
       * Tests if a value is valid, can be used to determine what a set
       * operation would return without actually modifying the model.
       * @param value The value to test.
       */
      virtual QValidator::State test(const Type& value) const;

      /**
       * Sets the value. By default this operation is a no-op that always
       * returns <i>QValidator::State::Invalid</i>.
       */
      virtual QValidator::State set(const Type& value);

      /** Connects a slot to the UpdateSignal. */
      virtual boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const = 0;

    protected:

      /** Constructs an empty model. */
      ValueModel() = default;

    private:
      ValueModel(const ValueModel&) = delete;
      ValueModel& operator =(const ValueModel&) = delete;
      AnyRef get_ref() const override;
      QValidator::State test_ref(AnyRef value) const override;
      QValidator::State set_ref(AnyRef value) override;
      boost::signals2::connection connect_update_signal_ref(
        const AnyValueModel::UpdateSignal::slot_type& slot) const override;
  };

  template<typename T>
  QValidator::State ValueModel<T>::test(const Type& value) const {
    return QValidator::State::Invalid;
  }

  template<typename T>
  QValidator::State ValueModel<T>::set(const Type& value) {
    return QValidator::State::Invalid;
  }

  template<typename T>
  AnyRef ValueModel<T>::get_ref() const {
    return get();
  }

  template<typename T>
  QValidator::State ValueModel<T>::test_ref(AnyRef value) const {
    if(value.get_type() != typeid(T)) {
      return QValidator::State::Invalid;
    }
    return test(any_cast<T>(value));
  }

  template<typename T>
  QValidator::State ValueModel<T>::set_ref(AnyRef value) {
    if(value.get_type() != typeid(T)) {
      return QValidator::State::Invalid;
    }
    return set(any_cast<T>(value));
  }

  template<typename T>
  boost::signals2::connection ValueModel<T>::connect_update_signal_ref(
      const AnyValueModel::UpdateSignal::slot_type& slot) const {
    return connect_update_signal([=] (const Type& current) {
      slot(current);
    });
  }
}

namespace Beam::Serialization {
  template<typename T>
  struct IsStructure<Spire::ValueModel<T>> : std::false_type {};

  template<typename T>
  struct Send<Spire::ValueModel<T>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const Spire::ValueModel<T>& value) const {
      shuttle.Shuttle(name, value.get());
    }
  };

  template<typename T>
  struct Receive<Spire::ValueModel<T>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Spire::ValueModel<T>& value) const {
      auto field = T();
      shuttle.Shuttle(name, field);
      value.set(field);
    }
  };
}

#endif
