#ifndef SPIRE_BLOTTER_CONTROLLER_HPP
#define SPIRE_BLOTTER_CONTROLLER_HPP
#include "Spire/Blotter/BlotterWindow.hpp"

namespace Spire {

  /** Controls a blotter's window. */
  class BlotterController {
    public:

      /**
       * Constructs a BlotterController.
       * @param model The controlled blotter.
       * @param collection The blotters available for linking.
       * @param cancel_bindings The user's cancel key bindings.
       * @param position_bindings The user's position key bindings.
       */
      BlotterController(std::shared_ptr<BlotterModel> model,
        std::shared_ptr<BlotterCollection> collection,
        std::shared_ptr<CancelKeyBindingsModel> cancel_bindings,
        std::shared_ptr<PositionKeyBindingsModel> position_bindings);

      ~BlotterController();

      /** Returns the controlled blotter. */
      const std::shared_ptr<BlotterModel>& get_model() const;

      /** Returns whether the window is open. */
      const std::shared_ptr<const BooleanModel>& is_open() const;

      /** Opens or raises the window. */
      BlotterWindow& open();

      /** Closes the window. */
      void close();

    private:
      class EventFilter;
      std::shared_ptr<BlotterModel> m_model;
      std::shared_ptr<BlotterCollection> m_collection;
      std::shared_ptr<CancelKeyBindingsModel> m_cancel_bindings;
      std::shared_ptr<PositionKeyBindingsModel> m_position_bindings;
      std::shared_ptr<LocalBooleanModel> m_is_open;
      std::shared_ptr<const BooleanModel> m_read_only_is_open;
      std::unique_ptr<BlotterWindow> m_window;
      std::unique_ptr<EventFilter> m_event_filter;

      BlotterController(const BlotterController&) = delete;
      BlotterController& operator =(const BlotterController&) = delete;
      void on_close();
  };
}

#endif
