#ifndef SPIRE_TOOLTIP_HPP
#define SPIRE_TOOLTIP_HPP
#include "Spire/Ui/InfoTip.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Represents a Spire-styled tooltip. */
  class Tooltip : public QObject {
    public:

      /**
       * Constructs a Tooltip.
       * @param text The Tooltip's label text.
       * @param parent The parent widget that when hovered will show the
       *                Tooltip.
       */
      Tooltip(QString text, QWidget* parent);

      /**
       * @param text The text label to display.
       */
      void set_label(const QString& text);

    private:
      TextBox* m_label;
      InfoTip* m_tooltip;
  };
}

#endif
