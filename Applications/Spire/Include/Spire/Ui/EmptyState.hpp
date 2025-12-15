#ifndef SPIRE_EMPTY_STATE_HPP
#define SPIRE_EMPTY_STATE_HPP
#include "Spire/Ui/ClickObserver.hpp"

namespace Spire {
  class OverlayPanel;

  /** Indicates that no results were found in a pop-up dropdown panel. */
  class EmptyState : public QWidget {
    public:

      /**
       * Constructs an EmptyState.
       * @param label The messge to display.
       * @param parent The parent widget that owns this EmptyState.
       */
      EmptyState(QString label, QWidget& parent);

      QSize sizeHint() const override;

    protected:
      bool event(QEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      OverlayPanel* m_panel;
      int m_panel_horizontal_border_width;
      ClickObserver m_click_observer;
      mutable boost::optional<QSize> m_size_hint;
      boost::signals2::scoped_connection m_panel_style_connection;

      void on_click();
      void on_panel_style();
  };
}

#endif
