#ifndef SPIRE_KEY_BINDINGS_PROFILE_HPP
#define SPIRE_KEY_BINDINGS_PROFILE_HPP
#include <filesystem>
#include <memory>
#include <vector>
#include "Spire/Canvas/Common/CanvasNode.hpp"
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

  /** Returns the list of built-in Lynx order task nodes. */
  std::vector<std::unique_ptr<CanvasNode>> make_lynx_order_task_nodes();

  /** Returns the list of built-in Match Now LP order task nodes. */
  std::vector<std::unique_ptr<CanvasNode>> make_matnlp_order_task_nodes();

  /** Returns the list of built-in Match Now MF order task nodes. */
  std::vector<std::unique_ptr<CanvasNode>> make_matnmf_order_task_nodes();

  /** Returns the list of built-in NEO order task nodes. */
  std::vector<std::unique_ptr<CanvasNode>> make_neoe_order_task_nodes();

  /** Returns the list of built-in Omega order task nodes. */
  std::vector<std::unique_ptr<CanvasNode>> make_omega_order_task_nodes();

  /** Returns the list of built-in Pure order task nodes. */
  std::vector<std::unique_ptr<CanvasNode>> make_pure_order_task_nodes();

  /** Returns the list of built-in TSX order task nodes. */
  std::vector<std::unique_ptr<CanvasNode>> make_tsx_order_task_nodes();

  /** Returns the full list of built-in order task nodes. */
  std::vector<std::unique_ptr<CanvasNode>> make_default_order_task_nodes();

  /**
   * Resets the order task arguments to their defaults.
   * @param order_task_arguments The list of order task arguments to reset.
   */
  void reset_order_task_arguments(
    OrderTaskArgumentsListModel& order_task_arguments);

  /**
   * Loads the key bindings profile found at a given path.
   * @param path The path to the directory containing the key bindings profile.
   */
  std::shared_ptr<KeyBindingsModel> load_key_bindings_profile(
    const std::filesystem::path& path);

  /**
   * Saves the key bindings profile to a given path.
   * @param path The path to the directory containing the key bindings profile.
   */
  void save_key_bindings_profile(
    const KeyBindingsModel& key_bindings, const std::filesystem::path& path);
}

#endif
