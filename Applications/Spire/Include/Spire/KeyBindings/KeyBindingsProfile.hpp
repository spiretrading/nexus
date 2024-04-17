#ifndef SPIRE_KEY_BINDINGS_PROFILE_HPP
#define SPIRE_KEY_BINDINGS_PROFILE_HPP
#include <filesystem>
#include <memory>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/KeyBindingsModel.hpp"

namespace Spire {

  /**
   * Loads the key bindings profile found at a given path.
   * @param path The path to the directory containing the key bindings profile.
   * @param markets The database of market definitions.
   */
  std::shared_ptr<KeyBindingsModel> load_key_bindings_profile(
    const std::filesystem::path& path, Nexus::MarketDatabase markets);

  /**
   * Saves the key bindings profile to a given path.
   * @param path The path to the directory containing the key bindings profile.
   */
  void save_key_bindings_profile(
    const KeyBindingsModel& key_bindings, const std::filesystem::path& path);
}

#endif
