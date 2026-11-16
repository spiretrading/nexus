#ifndef SPIRE_BLOTTER_MODEL_HPP
#define SPIRE_BLOTTER_MODEL_HPP
#include "Spire/Ui/TextBox.hpp"

namespace Spire {

  /** Manages a blotter's order execution tasks, orders, and positions. */
  class BlotterModel {
    public:
      virtual ~BlotterModel() = default;

      /** Returns the blotter's name. */
      virtual const std::shared_ptr<TextModel>& get_name() const = 0;

      /** Returns whether the blotter is active. */
      virtual const std::shared_ptr<BooleanModel>& is_active() const = 0;

      /** Returns whether the blotter is pinned for saving on exit. */
      virtual const std::shared_ptr<BooleanModel>& is_pinned() const = 0;
  };
}

#endif
