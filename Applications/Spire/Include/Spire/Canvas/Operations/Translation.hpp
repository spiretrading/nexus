#ifndef SPIRE_TRANSLATION_HPP
#define SPIRE_TRANSLATION_HPP
#include <memory>
#include <optional>
#include <typeinfo>
#include <type_traits>
#include <Aspen/Aspen.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {
namespace Details {
  template<typename T>
  struct IsBox : std::false_type {};

  template<typename T>
  struct IsBox<Aspen::Box<T>> : std::true_type {};
}

  /** Stores the result of a single CanvasNode translation. */
  class Translation {
    public:

      /**
       * Constructs a Translation for a weak observer reactor.
       * @param reactor The reactor that was translated.
       */
      template<typename R>
      Translation(Aspen::Weak<R> reactor);

      /**
       * Constructs a Translation for a shared reactor.
       * @param reactor The reactor that was translated.
       */
      template<typename R>
      Translation(Aspen::Shared<R> reactor);

      /**
       * Constructs a Translation for a reactor.
       * @param reactor The reactor that was translated.
       */
      template<typename R, typename=std::enable_if_t<Aspen::is_reactor_v<R>>>
      Translation(R&& reactor);

      /** Returns the type of value produced by the translated reactor. */
      const std::type_info& GetTypeInfo() const;

      /** Extracts the translated reactor. */
      template<typename T>
      T Extract() const;

      /**
       * Returns a Translation that converts the held reactor into a Shared
       * reactor.
       */
      Translation ToShared() const;

      /**
       * Returns a Translation that converts the held reactor into a Weak
       * reactor.
       */
      Translation ToWeak() const;

    private:
      struct BaseHolder : std::enable_shared_from_this<BaseHolder> {
        virtual ~BaseHolder() = default;
        virtual void ExtractVoidBox(void* destination) const = 0;
        virtual void ExtractBox(void* destination) const = 0;
        virtual void Extract(void* destination) const = 0;
        virtual std::shared_ptr<const BaseHolder> ToShared() const = 0;
        virtual std::shared_ptr<const BaseHolder> ToWeak() const = 0;
      };
      template<typename R>
      struct Holder final : BaseHolder {
        Aspen::Shared<R> m_reactor;

        template<typename F>
        Holder(F&& reactor);
        void ExtractVoidBox(void* destination) const override;
        void ExtractBox(void* destination) const override;
        void Extract(void* destination) const override;
        std::shared_ptr<const BaseHolder> ToShared() const override;
        std::shared_ptr<const BaseHolder> ToWeak() const override;
      };
      template<typename R>
      struct WeakHolder final : BaseHolder {
        Aspen::Weak<R> m_reactor;

        WeakHolder(Aspen::Weak<R> reactor);
        void ExtractVoidBox(void* destination) const override;
        void ExtractBox(void* destination) const override;
        void Extract(void* destination) const override;
        std::shared_ptr<const BaseHolder> ToShared() const override;
        std::shared_ptr<const BaseHolder> ToWeak() const override;
      };
      const std::type_info* m_type;
      std::shared_ptr<const BaseHolder> m_holder;

      Translation(const std::type_info& type,
        std::shared_ptr<const BaseHolder> holder);
  };

  template<typename R>
  Translation::Translation(Aspen::Weak<R> reactor)
    : m_type(&typeid(Aspen::reactor_result_t<Aspen::Weak<R>>)),
      m_holder(std::make_shared<WeakHolder<R>>(std::move(reactor))) {}

  template<typename R>
  Translation::Translation(Aspen::Shared<R> reactor)
    : m_type(&typeid(Aspen::reactor_result_t<Aspen::Shared<R>>)),
      m_holder(std::make_shared<Holder<R>>(std::move(reactor))) {}

  template<typename R, typename>
  Translation::Translation(R&& reactor)
    : m_type(&typeid(Aspen::reactor_result_t<R>)),
      m_holder(std::make_shared<Holder<std::decay_t<R>>>(
        std::forward<R>(reactor))) {}

  template<typename T>
  T Translation::Extract() const {
    auto destination = std::optional<T>();
    if constexpr(Details::IsBox<T>::value) {
      if constexpr(std::is_same_v<Aspen::reactor_result_t<T>, void>) {
        m_holder->ExtractVoidBox(&destination);
      } else {
        m_holder->ExtractBox(&destination);
      }
    } else {
      m_holder->Extract(&destination);
    }
    return std::move(*destination);
  }

  template<typename R>
  template<typename F>
  Translation::Holder<R>::Holder(F&& reactor)
    : m_reactor(std::forward<F>(reactor)) {}

  template<typename R>
  void Translation::Holder<R>::ExtractVoidBox(void* destination) const {
    static_cast<std::optional<Aspen::Box<void>>*>(destination)->emplace(
      m_reactor);
  }

  template<typename R>
  void Translation::Holder<R>::ExtractBox(void* destination) const {
    using Type = Aspen::reactor_result_t<R>;
    static_cast<std::optional<Aspen::Box<Type>>*>(destination)->emplace(
      m_reactor);
  }

  template<typename R>
  void Translation::Holder<R>::Extract(void* destination) const {
    static_cast<std::optional<Aspen::Shared<R>>*>(destination)->emplace(
      m_reactor);
  }

  template<typename R>
  std::shared_ptr<const Translation::BaseHolder>
      Translation::Holder<R>::ToShared() const {
    return shared_from_this();
  }

  template<typename R>
  std::shared_ptr<const Translation::BaseHolder>
      Translation::Holder<R>::ToWeak() const {
    return std::make_shared<WeakHolder<R>>(Aspen::Weak(m_reactor));
  }

  template<typename R>
  Translation::WeakHolder<R>::WeakHolder(Aspen::Weak<R> reactor)
    : m_reactor(std::move(reactor)) {}

  template<typename R>
  void Translation::WeakHolder<R>::ExtractVoidBox(void* destination) const {
    static_cast<std::optional<Aspen::Box<void>>*>(destination)->emplace(
      m_reactor);
  }

  template<typename R>
  void Translation::WeakHolder<R>::ExtractBox(void* destination) const {
    using Type = Aspen::reactor_result_t<Aspen::Weak<R>>;
    static_cast<std::optional<Aspen::Box<Type>>*>(destination)->emplace(
      m_reactor);
  }

  template<typename R>
  void Translation::WeakHolder<R>::Extract(void* destination) const {
    static_cast<std::optional<Aspen::Weak<R>>*>(destination)->emplace(
      m_reactor);
  }

  template<typename R>
  std::shared_ptr<const Translation::BaseHolder>
      Translation::WeakHolder<R>::ToShared() const {
    using Type = Aspen::reactor_result_t<R>;
    auto reactor = m_reactor.lock();
    if(reactor.has_value()) {
      return std::make_shared<Holder<R>>(std::move(*reactor));
    } else {
      if constexpr(Details::IsBox<R>::value) {
        try {
          return std::make_shared<Holder<Aspen::Box<Type>>>(Aspen::box(
            Aspen::constant(m_reactor.eval())));
        } catch(...) {
          return std::make_shared<Holder<Aspen::Box<Type>>>(Aspen::box(
            Aspen::throws<Type>(std::current_exception())));
        }
      } else {
        try {
          return std::make_shared<Holder<Aspen::Constant<Type>>>(
            Aspen::constant(m_reactor.eval()));
        } catch(...) {
          return std::make_shared<Holder<Aspen::Throw<Type>>>(
            Aspen::throws<Type>(std::current_exception()));
        }
      }
    }
  }

  template<typename R>
  std::shared_ptr<const Translation::BaseHolder>
      Translation::WeakHolder<R>::ToWeak() const {
    return shared_from_this();
  }
}

#endif
