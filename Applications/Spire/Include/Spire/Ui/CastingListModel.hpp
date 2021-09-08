#ifndef SPIRE_CASTING_LIST_MODEL_HPP
#define SPIRE_CASTING_LIST_MODEL_HPP
#include "Spire/Ui/ListModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Provides a type-safe view over a ListModel by automatically casting values
   * to and from a ListModel.
   * @param <T> The type to cast values read from the ListModel.
   */
  template<typename T>
  class CastingListModel {
    public:
      using AddOperation = ListModel::AddOperation;
      using RemoveOperation = ListModel::RemoveOperation;
      using MoveOperation = ListModel::MoveOperation;
      using UpdateOperation = ListModel::UpdateOperation;
      using Operation = ListModel::Operation;
      using Transaction = ListModel::Transaction;
      using OperationSignal = ListModel::OperationSignal;

      /** The type to cast values read from the ListModel. */
      using Type = T;

      /**
       * Constructs a CastingListModel.
       * @param source The ListModel to automatically cast.
       */
      CastingListModel(std::shared_ptr<ListModel> source);

      /** Returns the source model. */
      std::shared_ptr<const ListModel> get_source() const;

      /** Returns the source model. */
      const std::shared_ptr<ListModel>& get_source();

      /** Returns the number of items in the model. */
      int get_size() const;

      /**
       * Returns the value at a specified index.
       * @throws <code>std::out_of_range</code> iff index is out of range.
       */
      const Type& get(int index) const;

      /**
       * Sets the value at a specified index.
       * @param index - The index to set.
       * @param value - The value to set at the specified index.
       * @return The state of the value at the <i>index</i>, or
       *         <code>QValidator::State::Invalid</code> iff row or column is
       *         out of range.
       */
      QValidator::State set(int index, const Type& value);

      /** Connects a slot to the OperationSignal. */
      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const;

    private:
      std::shared_ptr<ListModel> m_source;
  };

  template<typename T>
  CastingListModel<T>::CastingListModel(std::shared_ptr<ListModel> source)
    : m_source(std::move(source)) {}

  template<typename T>
  std::shared_ptr<const ListModel> CastingListModel<T>::get_source() const {
    return m_source;
  }

  template<typename T>
  const std::shared_ptr<ListModel>& CastingListModel<T>::get_source() {
    return m_source;
  }

  template<typename T>
  int CastingListModel<T>::get_size() const {
    return m_source->get_size();
  }

  template<typename T>
  const typename CastingListModel<T>::Type&
      CastingListModel<T>::get(int index) const {
    return m_source->get<Type>(index);
  }

  template<typename T>
  QValidator::State CastingListModel<T>::set(int index, const Type& value) {
    return m_source->set(index, value);
  }

  template<typename T>
  boost::signals2::connection CastingListModel<T>::connect_operation_signal(
      const OperationSignal::slot_type& slot) const {
    return m_source->connect_operation_signal(slot);
  }
}

#endif
