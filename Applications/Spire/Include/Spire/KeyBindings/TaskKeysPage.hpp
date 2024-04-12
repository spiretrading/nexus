#ifndef SPIRE_TASK_KEYS_PAGE_HPP
#define SPIRE_TASK_KEYS_PAGE_HPP
#include <QWidget>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/KeyBindingsModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Implements a widget for the task key bindings. */
  class TaskKeysPage : public QWidget {
    public:

      /**
       * Constructs a TaskKeysPage.
       * @param key_bindings The KeyBindingsModel storing all user interactions.
       * @param parent The parent widget.
       */
      TaskKeysPage(std::shared_ptr<KeyBindingsModel> key_bindings,
        Nexus::DestinationDatabase destinations, Nexus::MarketDatabase markets,
        QWidget* parent = nullptr);

      /** Returns the key bindings being displayed. */
      const std::shared_ptr<KeyBindingsModel>& get_key_bindings() const;

    private:
      std::shared_ptr<KeyBindingsModel> m_key_bindings;
  };
}

#endif
