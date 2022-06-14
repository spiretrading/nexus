#ifndef SPIRE_COMPOSITE_MODEL_HPP
#define SPIRE_COMPOSITE_MODEL_HPP
#include <utility>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {
namespace Details {
  template<typename T, typename R>
  struct model_type {
    using type =
      std::remove_cvref_t<decltype(std::declval<T>().*std::declval<R>())>;
  };
  template<typename T, typename R>
  using model_type_t = typename model_type<T, R>::type;
}

  template<typename T>
  class CompositeModel : public ValueModel<T> {
    public:
      using Type = typename ValueModel<T>::Type;

      using UpdateSignal = typename ValueModel<T>::UpdateSignal;

      template<typename R>
      void define(R accessor,
        std::shared_ptr<ValueModel<Details::model_type_t<T, R>>> model);

      template<typename R>
      std::shared_ptr<ValueModel<Details::model_type_t<T, R>>> access(
        R accessor);

      const Type& get() const override;

      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      Type m_value;
      
  };

  template<typename T>
  template<typename R>
  void CompositeModel<T>::define(R accessor,
      std::shared_ptr<ValueModel<Details::model_type_t<T, R>>> model) {
  }

  template<typename T>
  template<typename R>
  std::shared_ptr<ValueModel<Details::model_type_t<T, R>>>
      CompositeModel<T>::access(R accessor) {
    return nullptr;
  }

  template<typename T>
  const typename CompositeModel<T>::Type& CompositeModel<T>::get() const {
    return m_value;
  }

  template<typename T>
  boost::signals2::connection CompositeModel<T>::connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return {};
  }
}

#endif
