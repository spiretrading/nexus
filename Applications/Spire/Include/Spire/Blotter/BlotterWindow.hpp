#ifndef SPIRE_BLOTTER_WINDOW_HPP
#define SPIRE_BLOTTER_WINDOW_HPP
#include <memory>
#include <boost/signals2/connection.hpp>
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

    private:
      std::shared_ptr<BlotterModel> m_blotter;
      boost::signals2::scoped_connection m_name_connection;

      void on_name_update(const QString& name);
  };
}

#endif
