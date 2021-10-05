#ifndef SPIRE_CAST_LIST_MODEL_HPP
#define SPIRE_CAST_LIST_MODEL_HPP
#include "Spire/Ui/ListModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Adapts an existing ListModel acting as a source, into a list of homogeneous
   * values.
   * If the source is uniquely owned then all operations are guaranteed to be
   * type-safe, otherwise operations may result in a std::bad_cast being raised.
   * @param <T> The type of values to cast to/from the source model.
   */
  template<typename T>
  class CastListModel : public ListModel {
    public:

      /** The type of values to cast to/from the source model. */
      using Type = T;

      /**
       * Constructs a CastListModel.
       * @param source The model to cast values to and from.
       */
      explicit CastListModel(std::shared_ptr<ListModel> source);

      int get_size() const override;

      const Type& get(int index) const;

      QValidator::State set(int index, const Type& value);

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<ListModel> m_source;

      const std::any& at(int index) const override;
      QValidator::State set(int index, const std::any& value) override;
  };

  template<typename T>
  CastListModel<T>::CastListModel(std::shared_ptr<ListModel> source)
    : m_source(std::move(source)) {}

  template<typename T>
  int CastListModel<T>::get_size() const {
    return m_source->get_size();
  }

  template<typename T>
  const typename CastListModel<T>::Type& CastListModel<T>::get(int index)
      const {
    return m_source->get<Type>(index);
  }

  template<typename T>
  QValidator::State CastListModel<T>::set(int index, const Type& value) {
    return m_source->set(index, value);
  }

  template<typename T>
  boost::signals2::connection CastListModel<T>::connect_operation_signal(
      const OperationSignal::slot_type& slot) const {
    return m_source->connect_operation_signal(slot);
  }

  template<typename T>
  const std::any& CastListModel<T>::at(int index) const {
    return m_source->at(index);
  }

  template<typename T>
  QValidator::State CastListModel<T>::set(int index, const std::any& value) {
    if(value.type() == typeid(Type)) {
      return m_source->set(index, value);
    }
    return QValidator::Invalid;
  }
}

#endif
