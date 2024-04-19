#ifndef SPIRE_KEY_BINDINGS_PROFILE_HPP
#define SPIRE_KEY_BINDINGS_PROFILE_HPP
#include <filesystem>
#include <memory>
#include <vector>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/KeyBindingsModel.hpp"

namespace Spire {

  /** Returns the list of built-in ASX order task nodes. */
  std::vector<std::unique_ptr<CanvasNode>> make_asx_order_task_nodes();

  /** Returns the list of built-in CXA order task nodes. */
  std::vector<std::unique_ptr<CanvasNode>> make_cxa_order_task_nodes();

  /** Returns the list of built-in Alpha order task nodes. */
  std::vector<std::unique_ptr<CanvasNode>> make_alpha_order_task_nodes();

  /** Returns the list of built-in CHIX order task nodes. */
  std::vector<std::unique_ptr<CanvasNode>> make_chix_order_task_nodes();

  /** Returns the list of built-in CSE order task nodes. */
  std::vector<std::unique_ptr<CanvasNode>> make_cse_order_task_nodes();

  /** Returns the list of built-in CSE2 order task nodes. */
  std::vector<std::unique_ptr<CanvasNode>> make_cse2_order_task_nodes();

  /** Returns the list of built-in CX2 order task nodes. */
  std::vector<std::unique_ptr<CanvasNode>> make_cx2_order_task_nodes();

  /** Returns the full list of built-in order task nodes. */
  std::vector<std::unique_ptr<CanvasNode>> make_default_order_task_nodes();

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
