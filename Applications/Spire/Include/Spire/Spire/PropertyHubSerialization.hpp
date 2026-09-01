#ifndef SPIRE_PROPERTY_HUB_SERIALIZATION_HPP
#define SPIRE_PROPERTY_HUB_SERIALIZATION_HPP
#include <string>
#include <Beam/Serialization/TypeRegistry.hpp>
#include "Nexus/Definitions/Ticker.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

namespace Spire {
  BEAM_REGISTER_TYPES(RegisterPropertyHubTypes,
    (LocalValueModel<int>, "Spire.IntegerProperty"),
    (LocalValueModel<std::string>, "Spire.StringProperty"),
    (LocalValueModel<Nexus::Ticker>, "Spire.TickerProperty"));
}

#endif
