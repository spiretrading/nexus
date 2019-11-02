#ifndef SPIRE_TRANSLATION_HPP
#define SPIRE_TRANSLATION_HPP
#include <memory>
#include <optional>
#include <typeinfo>
#include <type_traits>
#include <Aspen/Aspen.hpp>
#include <Beam/Queues/Publisher.hpp>
#include <Nexus/OrderExecutionService/Order.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {
namespace Details {
  template<typename T>
  struct IsBox : std::false_type {};

  template<typename T>
  struct IsBox<Aspen::Box<T>> : std::true_type {};

  template<typename R>
  struct IsShared : std::false_type {};

  template<typename R>
  struct IsShared<Aspen::Shared<R>> : std::true_type {};
}

  /**
   * Stores the result of a single CanvasNode translation.
   */
  class Translation {
    public:
      template<typename R>
      Translation(Aspen::Shared<R> reactor);

      template<typename T>
      Translation(Aspen::Box<T> reactor);

      template<typename R, typename=std::enable_if_t<Aspen::is_reactor_v<R>>>
      Translation(R&& reactor);

      const std::type_info& GetTypeInfo() const;

      const Beam::Publisher<const Nexus::OrderExecutionService::Order*>*
        GetPublisher() const;

      template<typename T>
      T Extract() const;

    private:
      struct BaseHolder {
        virtual ~BaseHolder() = default;
        virtual void ExtractBox(void* destination) const = 0;
        virtual void Extract(void* destination) const = 0;
      };
      template<typename R>
      struct SharedHolder final : BaseHolder {
        Aspen::Shared<R> m_reactor;

        SharedHolder(Aspen::Shared<R> reactor);
        void ExtractBox(void* destination) const override;
        void Extract(void* destination) const override;
      };
      template<typename T>
      struct BoxHolder final : BaseHolder {
        Aspen::Box<T> m_reactor;

        BoxHolder(Aspen::Box<T> reactor);
        void ExtractBox(void* destination) const override;
        void Extract(void* destination) const override;
      };
      std::shared_ptr<BaseHolder> m_holder;
  };

  template<typename R>
  Translation::Translation(Aspen::Shared<R> reactor)
    : m_holder(std::make_shared<SharedHolder<R>>(std::move(reactor))) {}

  template<typename T>
  Translation::Translation(Aspen::Box<T> reactor)
    : m_holder(std::make_shared<BoxHolder<T>>(std::move(reactor))) {}

  template<typename R, typename>
  Translation::Translation(R&& reactor)
    : Translation(Aspen::Shared(std::forward<R>(reactor))) {}

  template<typename T>
  T Translation::Extract() const {
    static_assert(Details::IsBox<T>::value || Details::IsShared<T>::value);
    auto destination = std::optional<T>();
    if constexpr(Details::IsBox<T>::value) {
      m_holder->ExtractBox(&destination);
    } else if constexpr(Details::IsShared<T>::value) {
      m_holder->Extract(&destination);
    }
    return std::move(*destination);
  }

  template<typename R>
  Translation::SharedHolder<R>::SharedHolder(Aspen::Shared<R> reactor)
    : m_reactor(std::move(reactor)) {}

  template<typename R>
  void Translation::SharedHolder<R>::ExtractBox(void* destination) const {
    using Type = Aspen::reactor_result_t<Aspen::Shared<R>>;
    static_cast<std::optional<Aspen::Box<Type>>*>(destination)->emplace(
      Aspen::Box(std::move(m_reactor)));
  }

  template<typename R>
  void Translation::SharedHolder<R>::Extract(void* destination) const {
    static_cast<std::optional<Aspen::Shared<R>>*>(destination)->emplace(
      std::move(m_reactor));
  }

  template<typename T>
  Translation::BoxHolder<T>::BoxHolder(Aspen::Box<T> reactor)
    : m_reactor(std::move(reactor)) {}

  template<typename T>
  void Translation::BoxHolder<T>::ExtractBox(void* destination) const {
    Extract(destination);
  }

  template<typename T>
  void Translation::BoxHolder<T>::Extract(void* destination) const {
    static_cast<std::optional<Aspen::Box<T>>*>(destination)->emplace(
      std::move(m_reactor));
  }
}

#endif
