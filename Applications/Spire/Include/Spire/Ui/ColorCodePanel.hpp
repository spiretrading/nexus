#ifndef SPIRE_COLOR_CODE_PANEL_HPP
#define SPIRE_COLOR_CODE_PANEL_HPP
#include <memory>
#include <QColor>
#include <QStackedWidget>
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Selects the alpha component. */
  using Alpha = StateSelector<void, struct AlphaSelectorTag>;
}

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

      QSize sizeHint() const override;

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      struct ColorCodeValueModel;
      std::shared_ptr<ColorCodeValueModel> m_current;
      DropDownBox* m_color_format_box;
      QStackedWidget* m_color_input;
      PercentBox* m_alpha_box;
      bool m_is_alpha_visible;
      mutable boost::optional<QSize> m_size_hint;
      boost::signals2::scoped_connection m_style_connection;

      void update_layout();
      void on_mode_current(const boost::optional<int>& current);
      void on_alpha_style();
  };
}

#endif
