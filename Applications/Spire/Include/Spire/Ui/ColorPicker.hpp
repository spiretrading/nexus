#ifndef SPIRE_COLOR_PICKER_HPP
#define SPIRE_COLOR_PICKER_HPP
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

namespace Spire {
  class OverlayPanel;
  class Slider2D;
  class Slider;

  /** A ValueModel over a QColor. */
  using ColorModel = ValueModel<QColor>;

  /** A LocalValueModel over a QColor. */
  using LocalColorModel = LocalValueModel<QColor>;

  /**
   * Displays a panel where the user can browse and select a color.
   */
  class ColorPicker : public QWidget {
    public:

      /** Specifies the visibility and feature set of the ColorPicker. */
      enum class Preset {

        /** The core color selection functionality. */
        BASIC,

        /** The core functionality and alpha transparency selection. */
        WITH_TRANSPARENCY,

        /** Full set of color selection features. */
        ADVANCED
      };

      /**
       * Constructs a ColorPicker with a default local model,
       * a predefined set of colors and the basic preset.
       * @param parent The parent widget.
       */
      explicit ColorPicker(QWidget& parent);

      /**
       * Constructs a ColorPicker with a default local model and
       * a predefined set of colors.
       * @param preset The visibility and feature preset.
       * @param parent The parent widget.
       */
      ColorPicker(Preset preset, QWidget& parent);

      /**
       * Constructs a ColorPicker with a predefined set of colors and
       * the basic preset.
       * @param current The model used for the current color.
       * @param parent The parent widget.
       */
      ColorPicker(std::shared_ptr<ColorModel> current, QWidget& parent);

      /**
       * Constructs a ColorPicker with a predefined set of colors.
       * @param current The model used for the current color.
       * @param preset The visibility and feature preset.
       * @param parent The parent widget.
       */
      ColorPicker(std::shared_ptr<ColorModel> current, Preset preset,
        QWidget& parent);

      /**
       * Constructs a ColorPicker with the basic preset.
       * @param current The model used for the current color.
       * @param palette The list model which holds a list of colors.
       * @param parent The parent widget.
       */
      ColorPicker(std::shared_ptr<ColorModel> current,
        std::shared_ptr<ListModel<QColor>> palette, QWidget& parent);

      /**
       * Constructs a ColorPicker.
       * @param current The model used for the current color.
       * @param palette The list model which holds a list of colors.
       * @param preset The visibility and feature preset.
       * @param parent The parent widget.
       */
      ColorPicker(std::shared_ptr<ColorModel> current,
        std::shared_ptr<ListModel<QColor>> palette, Preset preset,
        QWidget& parent);

      /** Returns the current color model. */
      const std::shared_ptr<ColorModel>& get_current() const;

      /** Returns the list of colors. */
      const std::shared_ptr<ListModel<QColor>>& get_palette() const;

      /** Returns the preset. */
      Preset get_preset() const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      struct ColorPickerModel;
      std::shared_ptr<ColorPickerModel> m_model;
      std::shared_ptr<ListModel<QColor>> m_palette;
      Preset m_preset;
      OverlayPanel* m_panel;
      Slider2D* m_color_spectrum;
      Slider* m_alpha_slider;
      QColor m_last_color;
      int m_panel_horizontal_spacing;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_style_connection;
      boost::signals2::scoped_connection m_panel_style_connection;

      void on_current(const QColor& current);
      void on_panel_style();
  };
}

#endif
