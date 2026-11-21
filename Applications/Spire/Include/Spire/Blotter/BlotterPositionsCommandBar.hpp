#ifndef SPIRE_BLOTTER_POSITIONS_COMMAND_BAR_HPP
#define SPIRE_BLOTTER_POSITIONS_COMMAND_BAR_HPP
#include <Beam/SignalHandling/ConnectionGroup.hpp>
#include "Spire/KeyBindings/PositionKeyBindingsModel.hpp"

namespace Spire {
  class Button;

  /** Displays commands for a blotter's positions. */
  class BlotterPositionsCommandBar : public QWidget {
    public:

      /** Signals a request to flatten the selected positions. */
      using FlattenSignal = Signal<void ()>;

      /** Signals a request to flatten all open positions. */
      using FlattenAllSignal = Signal<void ()>;

      /** Enumerates the available flatten commands. */
      enum class FlattenState {

        /** Neither command is available. */
        NONE,

        /** Only Flatten All is available. */
        ALL,

        /** Both Flatten and Flatten All are available. */
        SELECTED_AND_ALL
      };

      /**
       * Constructs a BlotterPositionsCommandBar.
       * @param can_flatten The available flatten commands.
       * @param bindings The user's position key bindings.
       * @param parent The parent widget.
       */
      BlotterPositionsCommandBar(
        std::shared_ptr<ValueModel<FlattenState>> can_flatten,
        std::shared_ptr<PositionKeyBindingsModel> bindings,
        QWidget* parent = nullptr);

      /** Connects a slot to the FlattenSignal. */
      boost::signals2::connection connect_flatten_signal(
        const FlattenSignal::slot_type& slot) const;

      /** Connects a slot to the FlattenAllSignal. */
      boost::signals2::connection connect_flatten_all_signal(
        const FlattenAllSignal::slot_type& slot) const;

    private:
      mutable FlattenSignal m_flatten_signal;
      mutable FlattenAllSignal m_flatten_all_signal;
      std::shared_ptr<ValueModel<FlattenState>> m_can_flatten;
      std::shared_ptr<PositionKeyBindingsModel> m_bindings;
      Button* m_flatten;
      Button* m_flatten_all;
      Beam::ConnectionGroup m_connections;

      void on_update(FlattenState state);
  };
}

#endif
