#ifndef SPIRE_BLOTTER_WINDOW_HPP
#define SPIRE_BLOTTER_WINDOW_HPP
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Displays a blotter. */
  class BlotterWindow : public Window {
    public:

      /**
       * Constructs a BlotterWindow.
       * @param model The blotter to display.
       * @param parent The parent widget.
       */
      BlotterWindow(
        std::shared_ptr<BlotterModel> model, QWidget* parent = nullptr);

      /** Returns the displayed blotter's model. */
      const std::shared_ptr<BlotterModel>& get_model() const;

    private:
      std::shared_ptr<BlotterModel> m_model;
      boost::signals2::scoped_connection m_name_connection;

      void on_name(const QString& name);
  };
}

#endif
