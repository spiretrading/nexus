#ifndef SPIRE_BLOTTER_WINDOW_HPP
#define SPIRE_BLOTTER_WINDOW_HPP
#include <memory>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Displays the blotter window. */
  class BlotterWindow : public Window {
    public:

      /**
       * Constructs a BlotterWindow.
       * @param blotter The blotter to represent.
       * @param parent The parent widget.
       */
      explicit BlotterWindow(
        std::shared_ptr<BlotterModel> blotter, QWidget* parent = nullptr);
  };
}

#endif
