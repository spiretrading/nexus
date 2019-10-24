#ifndef SPIRE_TRANSLATION_HPP
#define SPIRE_TRANSLATION_HPP
#include <memory>
#include <typeinfo>
#include <type_traits>
#include <Aspen/Aspen.hpp>
#include <Beam/Queues/Publisher.hpp>
#include <Nexus/OrderExecutionService/Order.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

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
  };

  template<typename R, typename>
  Translation::Translation(R&& reactor) {}

  template<typename T>
  T Translation::Extract() const {
    return *static_cast<const T*>(nullptr);
  }
}

#endif
