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
}

  /**
   * Stores the result of a single CanvasNode translation.
   */
  class Translation {
    public:
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
        virtual void ExtractBox(void* destination) = 0;
        virtual void Extract(void* destination) const = 0;
      };
      template<typename T>
      struct BoxHolder final : BaseHolder {
        Aspen::Box<T> m_reactor;

        BoxHolder(Aspen::Box<T> reactor);
        void ExtractBox(void* destination) override;
        void Extract(void* destination) const override;
      };
      template<typename T>
      struct Holder final : BaseHolder {
        T m_reactor;

        Holder(T reactor);
        void ExtractBox(void* destination) override;
        void Extract(void* destination) const override;
      };
      std::shared_ptr<BaseHolder> m_holder;
  };

  template<typename R, typename>
  Translation::Translation(R&& reactor) {
    if constexpr(Details::IsBox<std::decay_t<R>>::value) {
      m_holder = std::make_shared<BoxHolder<typename R::Type>>(
        std::forward<R>(reactor));
    } else {
      m_holder = std::make_shared<Holder<std::decay_t<R>>>(
        std::forward<R>(reactor));
    }
  }

  template<typename T>
  T Translation::Extract() const {
    auto destination = std::optional<T>();
    if constexpr(Details::IsBox<T>::value) {
      m_holder->ExtractBox(&destination);
    } else {
      m_holder->Extract(&destination);
    }
    return std::move(*destination);
  }

  template<typename T>
  Translation::BoxHolder<T>::BoxHolder(Aspen::Box<T> reactor)
    : m_reactor(std::move(reactor)) {}

  template<typename T>
  void Translation::BoxHolder<T>::ExtractBox(void* destination) {
    Extract(destination);
  }

  template<typename T>
  void Translation::BoxHolder<T>::Extract(void* destination) const {
    static_cast<std::optional<Aspen::Box<T>>*>(destination)->emplace(
      std::move(m_reactor));
  }

  template<typename T>
  Translation::Holder<T>::Holder(T reactor)
    : m_reactor(std::move(reactor)) {}

  template<typename T>
  void Translation::Holder<T>::ExtractBox(void* destination) {
    static_cast<std::optional<Aspen::Box<T>>*>(destination)->emplace(
      Aspen::Box(std::move(m_reactor)));
  }

  template<typename T>
  void Translation::Holder<T>::Extract(void* destination) const {
    static_cast<std::optional<T>*>(destination)->emplace(std::move(m_reactor));
  }
}

#endif
