#ifndef SPIRE_LIST_MODEL_HPP
#define SPIRE_LIST_MODEL_HPP
#include <boost/mpl/back.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <QValidator>
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Details {
  template<typename O>
  struct ListModelTransaction {
    std::vector<O> m_operations;
  };
}

  /**
   * Base class for a model over a table of values.
   * @param <T> The type of value stored by the list.
   */
  template<typename T>
  class ListModel {
    public:

      /** The type of value stored by the list. */
      using Type = T;

      /** Indicates a value was added to the model. */
      struct AddOperation {

        /** The index the value was inserted at. */
        int m_index;

        /** The value added. */
        Type m_value;
      };

      /** Indicates a value was removed from the model. */
      struct RemoveOperation {

        /** The index of the value removed. */
        int m_index;

        /** The value removed. */
        Type m_value;
      };

      /** Indicates a value was moved from one index to another. */
      struct MoveOperation {

        /** The index of the value that was moved. */
        int m_source;

        /** The index that the value was moved to. */
        int m_destination;
      };

      /** Indicates a value was updated. */
      struct UpdateOperation {

        /** The index of the updated value. */
        int m_index;

        /** The updated value. */
        Type m_value;

        /** The previous value. */
        Type m_previous_value;
      };

      /** Consolidates all basic operations. */
      using Operation = boost::make_recursive_variant<AddOperation,
        RemoveOperation, MoveOperation, UpdateOperation,
        Details::ListModelTransaction<boost::recursive_variant_>>;

      /**
       * An operation consisting of a list of sub-operations performed as
       * single transaction.
       */
      using Transaction =
        typename boost::mpl::back<typename Operation::types>::type;

      /**
       * Signals an operation was applied to this model.
       * @param operation The operation that was applied.
       */
      using OperationSignal = Signal<void (const Operation&)>;

      virtual ~ListModel() = default;

      /** Returns the number of values in the model. */
      virtual int get_size() const = 0;

      /**
       * Returns the value at a specified index.
       * @throws <code>std::out_of_range</code> iff index is out of range.
       */
      virtual const T& get(int index) const = 0;

      /**
       * Returns the state of the model, by default this is
       * <i>QValidator::State::Acceptable</i>
       */
      virtual QValidator::State get_state() const;

      /**
       * Applies an Operation to this model. By default this operation is a
       * no-op that always returns <i>QValidator::State::Invalid</i>.
       */
      virtual QValidator::State apply(const Operation& operation);

      /** Connects a slot to the OperationSignal. */
      virtual boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const = 0;

    protected:

      /** Constructs an empty model. */
      ListModel() = default;

    private:
      ListModel(const ListModel&) = delete;
      ListModel& operator =(const ListModel&) = delete;
  };

  template<typename T>
  QValidator::State ListModel<T>::get_state() const {
    return QValidator::State::Acceptable;
  }

  template<typename T>
  QValidator::State ListModel<T>::apply(const Operation& operation) {
    return QValidator::State::Invalid;
  }
}

#endif
