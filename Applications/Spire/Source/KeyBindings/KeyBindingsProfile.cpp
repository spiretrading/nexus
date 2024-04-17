#include "Spire/KeyBindings/KeyBindingsProfile.hpp"
#include <fstream>
#include <unordered_map>
#include <Beam/Collections/Enum.hpp>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include "Spire/LegacyUI/UISerialization.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Nexus;
using namespace Spire;

namespace {
  struct LegacyKeyBindings {
    struct TaskBinding {
      std::string m_name;
      std::shared_ptr<CanvasNode> m_node;
    };
    struct CancelBinding {
      BEAM_ENUM(Type,
        MOST_RECENT,
        MOST_RECENT_ASK,
        MOST_RECENT_BID,
        OLDEST,
        OLDEST_ASK,
        OLDEST_BID,
        ALL,
        ALL_ASKS,
        ALL_BIDS,
        CLOSEST_ASK,
        CLOSEST_BID,
        FURTHEST_ASK,
        FURTHEST_BID);
      std::string m_description;
      Type m_type;
    };
    std::unordered_map<MarketCode,
      std::unordered_map<QKeySequence, TaskBinding>> m_task_bindings;
    std::unordered_map<QKeySequence, CancelBinding> m_cancel_bindings;
    std::unordered_map<MarketCode, Quantity> m_default_quantities;
  };

  auto from_legacy(LegacyKeyBindings::CancelBinding::Type binding) {
    if(binding == LegacyKeyBindings::CancelBinding::Type::MOST_RECENT) {
      return CancelKeyBindingsModel::Operation::MOST_RECENT;
    } else if(
        binding == LegacyKeyBindings::CancelBinding::Type::MOST_RECENT_ASK) {
      return CancelKeyBindingsModel::Operation::MOST_RECENT_ASK;
    } else if(
        binding == LegacyKeyBindings::CancelBinding::Type::MOST_RECENT_BID) {
      return CancelKeyBindingsModel::Operation::MOST_RECENT_BID;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::OLDEST) {
      return CancelKeyBindingsModel::Operation::OLDEST;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::OLDEST_ASK) {
      return CancelKeyBindingsModel::Operation::OLDEST_ASK;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::OLDEST_BID) {
      return CancelKeyBindingsModel::Operation::OLDEST_BID;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::ALL) {
      return CancelKeyBindingsModel::Operation::ALL;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::ALL_ASKS) {
      return CancelKeyBindingsModel::Operation::ALL_ASKS;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::ALL_BIDS) {
      return CancelKeyBindingsModel::Operation::ALL_BIDS;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::CLOSEST_ASK) {
      return CancelKeyBindingsModel::Operation::CLOSEST_ASK;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::CLOSEST_BID) {
      return CancelKeyBindingsModel::Operation::CLOSEST_BID;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::FURTHEST_ASK) {
      return CancelKeyBindingsModel::Operation::FURTHEST_ASK;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::FURTHEST_BID) {
      return CancelKeyBindingsModel::Operation::FURTHEST_BID;
    }
    throw std::runtime_error("Invalid cancel binding.");
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<LegacyKeyBindings::TaskBinding> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, LegacyKeyBindings::TaskBinding& value,
        unsigned int version) {
      shuttle.Shuttle("name", value.m_name);
      shuttle.Shuttle("node", value.m_node);
    }
  };

  template<>
  struct Shuttle<LegacyKeyBindings::CancelBinding> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, LegacyKeyBindings::CancelBinding& value,
        unsigned int version) {
      shuttle.Shuttle("description", value.m_description);
      shuttle.Shuttle("type", value.m_type);
    }
  };

  template<>
  struct Shuttle<LegacyKeyBindings> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, LegacyKeyBindings& value, unsigned int version) {
      shuttle.Shuttle("task_bindings", value.m_task_bindings);
      shuttle.Shuttle("cancel_bindings", value.m_cancel_bindings);
      shuttle.Shuttle("default_quantities", value.m_default_quantities);
    }
  };
}

namespace {
  auto convert_legacy_key_bindings(
      const std::filesystem::path& path, MarketDatabase markets) {
    if(!std::filesystem::exists(path)) {
      throw std::runtime_error("key_bindings.dat not found.");
    }
    auto legacy_key_bindings = LegacyKeyBindings();
    try {
      auto reader =
        BasicIStreamReader<std::ifstream>(Initialize(path, std::ios::binary));
      auto buffer = SharedBuffer();
      reader.Read(Store(buffer));
      auto registry = TypeRegistry<BinarySender<SharedBuffer>>();
      RegisterSpireTypes(Store(registry));
      auto receiver = BinaryReceiver<SharedBuffer>(Ref(registry));
      receiver.SetSource(Ref(buffer));
      receiver.Shuttle(legacy_key_bindings);
    } catch(std::exception&) {
      throw std::runtime_error("Unable to load key bindings, using defaults.");
    }
    auto key_bindings = std::make_shared<KeyBindingsModel>(std::move(markets));
    for(auto& cancel_binding : legacy_key_bindings.m_cancel_bindings) {
      key_bindings->get_cancel_key_bindings()->get_binding(
        from_legacy(cancel_binding.second.m_type))->set(cancel_binding.first);
    }
    return key_bindings;
  }

  auto load_default_key_bindings(MarketDatabase markets) {
    return std::make_shared<KeyBindingsModel>(std::move(markets));
  }
}

std::shared_ptr<KeyBindingsModel> Spire::load_key_bindings_profile(
    const std::filesystem::path& path, MarketDatabase markets) {
  auto file_path = path / "key_bindings.json";
  if(!std::filesystem::exists(file_path)) {
    auto legacy_path = path / "key_bindings.dat";
    if(std::filesystem::exists(legacy_path)) {
      return convert_legacy_key_bindings(legacy_path, std::move(markets));
    }
    return load_default_key_bindings(std::move(markets));
  }
  return std::make_shared<KeyBindingsModel>(std::move(markets));
}

void Spire::save_key_bindings_profile(
    const KeyBindingsModel& key_bindings, const std::filesystem::path& path) {
}
