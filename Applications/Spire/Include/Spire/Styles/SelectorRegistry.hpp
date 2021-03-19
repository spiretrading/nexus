#ifndef SPIRE_STYLES_SELECTOR_REGISTRY_HPP
#define SPIRE_STYLES_SELECTOR_REGISTRY_HPP
#include <QWidget>
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /**
   * Keeps track of changes to all selectors among all widgets within a window.
   */
  class SelectorRegistry {
    public:

      /**
       * Returns the registry associated with a widget's window.
       * @param widget Any widget.
       * @return The SelectorRegistry associated with the <i>widget</i>'s
       *         window.
       */
      static SelectorRegistry& find(QWidget& widget);

      /**
       * Adds a widget to be monitored for changes to its selectors.
       * @param widget The widget to monitor.
       */
      static void add(QWidget& widget);

      /** Returns the top-most widget. */
      QWidget& get_root() const;

      /** Indicates a change in a widget's selector. */
      void notify();

    private:
      QWidget* m_root;
      std::unique_ptr<QObject> m_event_listener;

      explicit SelectorRegistry(QWidget& root);
      SelectorRegistry(const SelectorRegistry&) = delete;
      SelectorRegistry& operator =(const SelectorRegistry&) = delete;
  };
}

#endif
