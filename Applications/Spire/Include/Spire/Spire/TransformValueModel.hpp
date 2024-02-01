#ifndef SPIRE_TRANSFORM_VALUE_MODEL_HPP
#define SPIRE_TRANSFORM_VALUE_MODEL_HPP
#include <concepts>
#include <memory>
#include <type_traits>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

namespace Spire {
namespace Details {
  template<typename F>
  struct TransformValueModelCollapser {
    F m_f;

    auto operator ()(auto&& current, auto&& value) {
      return m_f(std::forward<decltype(value)>(value));
    }
  };

  template<typename F>
  TransformValueModelCollapser(F&&) ->
    TransformValueModelCollapser<std::remove_reference_t<F>>;
}

  /**
   * A ValueModel that transforms the value of a source model via a callable.
   * @param <T> The type of value to model.
   * @param <U> The type of the source's value.
   * @param <F> The type of callable used to transform a value of type <i>U</i>
   *            into a value of type <i>T</i>.
   * @param <G> The type of callable used to invert <i>F</i>.
   */
  template<typename T, typename U, typename F, typename G>
  class TransformValueModel : public ValueModel<T> {
    public:

      /** The type of value to model. */
      using Source = U;
      using Type = typename ValueModel<T>::Type;
      using UpdateSignal = typename ValueModel<T>::UpdateSignal;

      /**
       * Constructs a TransformValueModel.
       * @param source The source model being transformed.
       * @param f The callable used to perform the transformation.
       * @param g The callable used to invert <i>f</i>.
       */
      template<typename FF, std::invocable<Type> GG>
      TransformValueModel(
        std::shared_ptr<ValueModel<Source>> source, FF&& f, GG&& g)
        : TransformValueModel(std::move(source), std::forward<FF>(f),
            Details::TransformValueModelCollapser(std::forward<GG>(g))) {}

      /**
       * Constructs a TransformValueModel.
       * @param source The source model being transformed.
       * @param f The callable used to perform the transformation.
       * @param g The callable used to invert <i>f</i>.
       */
      template<typename FF, std::invocable<Source, Type> GG>
      TransformValueModel(
          std::shared_ptr<ValueModel<Source>> source, FF&& f, GG&& g)
          : m_source(std::move(source)),
            m_f(std::forward<FF>(f)),
            m_g(std::forward<GG>(g)),
            m_model(m_f(m_source->get())) {
        m_source->connect_update_signal(
          std::bind_front(&TransformValueModel::on_update, this));
      }

      QValidator::State get_state() const override;

      const Type& get() const override;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<ValueModel<Source>> m_source;
      F m_f;
      mutable G m_g;
      LocalValueModel<Type> m_model;

      void on_update(const Source& value);
  };

  template<typename T, typename F,
    std::invocable<std::invoke_result_t<F, typename T::Type>> G>
  TransformValueModel(std::shared_ptr<T>, F&&, G&&) ->
    TransformValueModel<std::invoke_result_t<F, typename T::Type>,
      typename T::Type, std::remove_reference_t<F>,
      Details::TransformValueModelCollapser<std::remove_reference_t<G>>>;

  template<typename T, typename F, std::invocable<
    typename T::Type, std::invoke_result_t<F, typename T::Type>> G>
  TransformValueModel(std::shared_ptr<T>, F&&, G&&) ->
    TransformValueModel<std::invoke_result_t<F, typename T::Type>,
      typename T::Type, std::remove_reference_t<F>, std::remove_reference_t<G>>;

  template<typename T, typename F, typename G>
  auto make_transform_value_model(std::shared_ptr<T> source, F&& f, G&& g) {
    using Model = decltype(TransformValueModel(
      std::move(source), std::forward<F>(f), std::forward<G>(g)));
    return std::make_shared<Model>(
      std::move(source), std::forward<F>(f), std::forward<G>(g));
  }

  template<typename T, typename U, typename F, typename G>
  QValidator::State TransformValueModel<T, U, F, G>::get_state() const {
    return m_source->get_state();
  }

  template<typename T, typename U, typename F, typename G>
  const typename TransformValueModel<T, U, F, G>::Type&
      TransformValueModel<T, U, F, G>::get() const {
    return m_model.get();
  }

  template<typename T, typename U, typename F, typename G>
  QValidator::State TransformValueModel<T, U, F, G>::test(
      const Type& value) const {
    try {
      return m_source->test(m_g(m_source->get(), value));
    } catch(const std::invalid_argument&) {
      return QValidator::State::Invalid;
    }
  }

  template<typename T, typename U, typename F, typename G>
  QValidator::State TransformValueModel<T, U, F, G>::set(const Type& value) {
    try {
      return m_source->set(m_g(m_source->get(), value));
    } catch(const std::invalid_argument&) {
      return QValidator::State::Invalid;
    }
  }

  template<typename T, typename U, typename F, typename G>
  boost::signals2::connection
      TransformValueModel<T, U, F, G>::connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const {
    return m_model.connect_update_signal(slot);
  }

  template<typename T, typename U, typename F, typename G>
  void TransformValueModel<T, U, F, G>::on_update(const Source& value) {
    m_model.set(m_f(value));
  }
}

#endif
