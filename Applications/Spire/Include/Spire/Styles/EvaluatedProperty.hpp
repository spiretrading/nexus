#ifndef SPIRE_STYLES_EVALUATED_PROPERTY_HPP
#define SPIRE_STYLES_EVALUATED_PROPERTY_HPP
#include <any>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <Beam/Utilities/Functional.hpp>
#include "Spire/Styles/ConstantExpression.hpp"
#include "Spire/Styles/Property.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Stores the evaluation of a Property. */
  class EvaluatedProperty {
    public:

      /**
       * Constructs an EvaluatedProperty.
       * @param property The property that was evaluated.
       * @param evaluation The evaluation of the property.
       */
      template<typename T, typename G>
      EvaluatedProperty(const BasicProperty<T, G>& property,
        typename BasicProperty<T, G>::Type evaluation);

      /**
       * Constructs an EvaluatedProperty.
       * @param evaluation The evaluation of the property.
       */
      template<typename P>
      explicit EvaluatedProperty(
        std::in_place_type_t<P> property_type, typename P::Type evaluation);

      /** Returns the underlying property's type. */
      std::type_index get_property_type() const;

      /**
       * Returns a Property whose Expression evaluates to the same evaluation
       * as <code>this</code>
       */
      Property as_property() const;

      /** Casts the evaluation to a specified type. */
      template<typename U>
      const U& as() const;

      /**
       * Applies a callable to the underlying property stored.
       * @param f The callable to apply.
       */
      template<typename F>
      decltype(auto) visit(F&& f) const;

      template<typename F, typename... G>
      decltype(auto) visit(F&& f, G&&... g) const;

      bool operator ==(const EvaluatedProperty& property) const;

      bool operator !=(const EvaluatedProperty& property) const;

    private:
      template<typename T>
      struct TypeExtractor {};
      template<typename T, typename U>
      struct TypeExtractor<Beam::TypeSequence<std::in_place_type_t<T>, U>> {
        using type = T;
      };
      template<typename T, typename U, typename V>
      struct TypeExtractor<Beam::TypeSequence<T, std::in_place_type_t<U>, V>> {
        using type = U;
      };
      struct BaseEntry {
        virtual ~BaseEntry() = default;
        virtual std::type_index get_property_type() const = 0;
        virtual Property as_property() const = 0;
        virtual const void* as(std::type_index type) const = 0;
        virtual bool operator ==(const BaseEntry& entry) const = 0;
        bool operator !=(const BaseEntry& entry) const;
      };
      template<typename P>
      struct Entry final : BaseEntry {
        using BasicProperty = P;
        typename BasicProperty::Type m_evaluation;

        Entry(typename BasicProperty::Type evaluation);
        std::type_index get_property_type() const override;
        Property as_property() const override;
        const void* as(std::type_index type) const override;
        bool operator ==(const BaseEntry& entry) const override;
      };
      std::shared_ptr<BaseEntry> m_entry;
  };

  template<typename T, typename G>
  EvaluatedProperty::EvaluatedProperty(const BasicProperty<T, G>& property,
    typename BasicProperty<T, G>::Type evaluation)
    : EvaluatedProperty(
        std::in_place_type<BasicProperty<T, G>>, std::move(evaluation)) {}

  template<typename P>
  EvaluatedProperty::EvaluatedProperty(
    std::in_place_type_t<P> property_type, typename P::Type evaluation)
    : m_entry(std::make_shared<Entry<P>>(std::move(evaluation))) {}

  template<typename U>
  const U& EvaluatedProperty::as() const {
    return *static_cast<const U*>(m_entry->as(typeid(U)));
  }

  template<typename F>
  decltype(auto) EvaluatedProperty::visit(F&& f) const {
    using Parameter = typename TypeExtractor<
      Beam::GetFunctionParameters<std::decay_t<F>>>::type;
    if constexpr(std::is_invocable_v<std::decay_t<F>>) {
      return std::forward<F>(f)();
    } else if(m_entry->get_property_type() == typeid(Parameter)) {
      return std::forward<F>(f)(
        std::in_place_type<Parameter>, as<typename Parameter::Type>());
    }
  }

  template<typename F, typename... G>
  decltype(auto) EvaluatedProperty::visit(F&& f, G&&... g) const {
    using Parameter = typename TypeExtractor<
      Beam::GetFunctionParameters<std::decay_t<F>>>::type;
    if(m_entry->get_property_type() == typeid(Parameter)) {
      return std::forward<F>(f)(
        std::in_place_type<Parameter>, as<typename Parameter::Type>());
    }
    return visit(std::forward<G>(g)...);
  }

  template<typename P>
  EvaluatedProperty::Entry<P>::Entry(typename BasicProperty::Type evaluation)
    : m_evaluation(std::move(evaluation)) {}

  template<typename P>
  std::type_index EvaluatedProperty::Entry<P>::get_property_type() const {
    return typeid(BasicProperty);
  }

  template<typename P>
  Property EvaluatedProperty::Entry<P>::as_property() const {
    return Property(BasicProperty(ConstantExpression(m_evaluation)));
  }

  template<typename P>
  const void* EvaluatedProperty::Entry<P>::as(std::type_index type) const {
    if(type != typeid(typename BasicProperty::Type)) {
      throw std::bad_any_cast();
    }
    return &m_evaluation;
  }

  template<typename P>
  bool EvaluatedProperty::Entry<P>::operator ==(const BaseEntry& entry) const {
    return get_property_type() == entry.get_property_type() &&
      m_evaluation ==
        static_cast<const Entry<BasicProperty>&>(entry).m_evaluation;
  }
}

#endif
