#ifndef SPIRE_COLOR_CODE_PANEL_HPP
#define SPIRE_COLOR_CODE_PANEL_HPP
#include <QBoxLayout>
#include <QLabel>
#include <QWidget>
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Displays a panel where the user can enter a color code using different
   * color formats.
   */
  class ColorCodePanel : public QWidget {
    public:

      /** Specifies the display mode. */
      enum class Mode {

        /** The color is displayed in hex rgb format. */
        HEX,

        /** The color is displayed in RGB format. */
        RGB,

        /** The color is displayed in HSB format. */
        HSB
      };

      /**
       * Constructs a ColorCodePanel using a LocalValueModel.
       * @param parent The parent widget.
       */
      explicit ColorCodePanel(QWidget* parent = nullptr);

      /**
       * Constructs a ColorCodePanel.
       * @param current The model used for the current color.
       * @param parent The parent widget.
       */
      explicit ColorCodePanel(std::shared_ptr<ValueModel<QColor>> current,
        QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<ValueModel<QColor>>& get_current() const;

      /** Returns the display mode. */
      Mode get_mode() const;

      /** Sets the display mode. */
      void set_mode(Mode mode);

    private:
      std::shared_ptr<ValueModel<QColor>> m_current;
      Mode m_mode;
  };
}

#endif