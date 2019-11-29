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
  struct IsSharedBox : std::false_type {};

  template<typename T>
  struct IsSharedBox<Aspen::SharedBox<T>> : std::true_type {};

  template<typename R>
  struct IsShared : std::false_type {};

  template<typename R>
  struct IsShared<Aspen::Shared<R>> : std::true_type {};

  template<typename R>
  struct IsWeak : std::false_type {};

  template<typename R>
  struct IsWeak<Aspen::Weak<R>> : std::true_type {};
}

  /** Stores the result of a single CanvasNode translation. */
  class Translation {
    public:

      /**
       * Constructs a Translation for a shared reactor.
       * @param reactor The reactor that was translated.
       */
      template<typename R>
      Translation(Aspen::Shared<R> reactor);

      /**
       * Constructs a Translation for a weak reactor.
       * @param reactor The reactor that was translated.
       */
      template<typename R>
      Translation(Aspen::Weak<R> reactor);

      /**
       * Constructs a Translation for a shared box reactor.
       * @param reactor The reactor that was translated.
       */
      template<typename T>
      Translation(Aspen::SharedBox<T> reactor);

      /**
       * Constructs a Translation for a boxed reactor.
       * @param reactor The reactor that was translated.
       */
      template<typename T>
      Translation(Aspen::Box<T> reactor);

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

    private:
      struct BaseHolder {
        virtual ~BaseHolder() = default;
        virtual void ExtractSharedVoidBox(void* destination) const = 0;
        virtual void ExtractSharedBox(void* destination) const = 0;
        virtual void Extract(void* destination) const = 0;
      };
      template<typename R>
      struct SharedHolder final : BaseHolder {
        Aspen::Shared<R> m_reactor;

        SharedHolder(Aspen::Shared<R> reactor);
        void ExtractSharedVoidBox(void* destination) const override;
        void ExtractSharedBox(void* destination) const override;
        void Extract(void* destination) const override;
      };
      template<typename R>
      struct WeakHolder final : BaseHolder {
        Aspen::Weak<R> m_reactor;

        WeakHolder(Aspen::Weak<R> reactor);
        void ExtractSharedVoidBox(void* destination) const override;
        void ExtractSharedBox(void* destination) const override;
        void Extract(void* destination) const override;
      };
      template<typename T>
      struct BoxHolder final : BaseHolder {
        Aspen::SharedBox<T> m_reactor;

        BoxHolder(Aspen::SharedBox<T> reactor);
        void ExtractSharedVoidBox(void* destination) const override;
        void ExtractSharedBox(void* destination) const override;
        void Extract(void* destination) const override;
      };
      const std::type_info* m_type;
      std::shared_ptr<BaseHolder> m_holder;
  };

  template<typename R>
  Translation::Translation(Aspen::Shared<R> reactor)
    : m_type(&typeid(Aspen::reactor_result_t<R>)),
      m_holder(std::make_shared<SharedHolder<R>>(std::move(reactor))) {}

  template<typename R>
  Translation::Translation(Aspen::Weak<R> reactor)
    : m_type(&typeid(Aspen::reactor_result_t<R>)),
      m_holder(std::make_shared<WeakHolder<R>>(std::move(reactor))) {}

  template<typename T>
  Translation::Translation(Aspen::SharedBox<T> reactor)
    : m_type(&typeid(T)),
      m_holder(std::make_shared<BoxHolder<T>>(std::move(reactor))) {}

  template<typename T>
  Translation::Translation(Aspen::Box<T> reactor)
    : m_type(&typeid(T)),
      m_holder(std::make_shared<BoxHolder<T>>(
        Aspen::Shared(std::move(reactor)))) {}

  template<typename R, typename>
  Translation::Translation(R&& reactor)
    : Translation(Aspen::Shared(std::forward<R>(reactor))) {}

  template<typename T>
  T Translation::Extract() const {
    static_assert(Details::IsSharedBox<T>::value ||
      Details::IsShared<T>::value || Details::IsWeak<T>::value);
    auto destination = std::optional<T>();
    if constexpr(Details::IsSharedBox<T>::value) {
      if constexpr(std::is_same_v<Aspen::reactor_result_t<T>, void>) {
        m_holder->ExtractSharedVoidBox(&destination);
      } else {
        m_holder->ExtractSharedBox(&destination);
      }
    } else if constexpr(Details::IsShared<T>::value ||
        Details::IsWeak<T>::value) {
      m_holder->Extract(&destination);
    }
    return std::move(*destination);
  }

  template<typename R>
  Translation::SharedHolder<R>::SharedHolder(Aspen::Shared<R> reactor)
    : m_reactor(std::move(reactor)) {}

  template<typename R>
  void Translation::SharedHolder<R>::ExtractSharedVoidBox(
      void* destination) const {
    static_cast<std::optional<Aspen::SharedBox<void>>*>(destination)->emplace(
      m_reactor);
  }

  template<typename R>
  void Translation::SharedHolder<R>::ExtractSharedBox(void* destination) const {
    using Type = Aspen::reactor_result_t<Aspen::Shared<R>>;
    static_cast<std::optional<Aspen::SharedBox<Type>>*>(destination)->emplace(
      m_reactor);
  }

  template<typename R>
  void Translation::SharedHolder<R>::Extract(void* destination) const {
    static_cast<std::optional<Aspen::Shared<R>>*>(destination)->emplace(
      m_reactor);
  }

  template<typename R>
  Translation::WeakHolder<R>::WeakHolder(Aspen::Weak<R> reactor)
    : m_reactor(std::move(reactor)) {}

  template<typename R>
  void Translation::WeakHolder<R>::ExtractSharedVoidBox(
      void* destination) const {
    static_cast<std::optional<Aspen::SharedBox<void>>*>(destination)->emplace(
      m_reactor);
  }

  template<typename R>
  void Translation::WeakHolder<R>::ExtractSharedBox(void* destination) const {
    using Type = Aspen::reactor_result_t<Aspen::Weak<R>>;
    static_cast<std::optional<Aspen::SharedBox<Type>>*>(destination)->emplace(
      m_reactor);
  }

  template<typename R>
  void Translation::WeakHolder<R>::Extract(void* destination) const {
    static_cast<std::optional<Aspen::Weak<R>>*>(destination)->emplace(
      m_reactor);
  }

  template<typename T>
  Translation::BoxHolder<T>::BoxHolder(Aspen::SharedBox<T> reactor)
    : m_reactor(std::move(reactor)) {}

  template<typename T>
  void Translation::BoxHolder<T>::ExtractSharedVoidBox(
      void* destination) const {
    if constexpr(std::is_same_v<T, void>) {
      Extract(destination);
    } else {
      static_cast<std::optional<Aspen::SharedBox<void>>*>(destination)->emplace(
        m_reactor);
    }
  }

  template<typename T>
  void Translation::BoxHolder<T>::ExtractSharedBox(void* destination) const {
    Extract(destination);
  }

  template<typename T>
  void Translation::BoxHolder<T>::Extract(void* destination) const {
    static_cast<std::optional<Aspen::SharedBox<T>>*>(destination)->emplace(
      m_reactor);
  }
}

#endif
