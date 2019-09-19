#ifndef SPIRE_TRANSLATION_HPP
#define SPIRE_TRANSLATION_HPP
#include <memory>
#include <typeinfo>
#include <type_traits>
#include <Aspen/Aspen.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {
  class Translation {
    public:

      template<typename R, typename=std::enable_if_t<Aspen::is_reactor_v<R>>>
      Translation(R&& reactor);

      const std::type_info& GetTypeInfo() const;

      template<typename T>
      T Extract() const;
  };
}

#endif
