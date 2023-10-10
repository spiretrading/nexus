#ifndef SPIRE_COLOR_PICKER_HPP
#define SPIRE_COLOR_PICKER_HPP
#include <memory>
#include <QColor>
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Displays a panel where the user can browse and select a color.
   */
  class ColorPicker : public QWidget {
    public:

      /**
       * Constructs a ColorPicker with a default local model and
       * a predefined set of colors.
       * @param parent The parent widget.
       */
      explicit ColorPicker(QWidget& parent);

      /**
       * Constructs a ColorPicker with a predefined set of colors.
       * @param current The model used for the current color.
       * @param parent The parent widget.
       */
      ColorPicker(std::shared_ptr<ValueModel<QColor>> current,
        QWidget& parent);

      /**
       * Constructs a ColorPicker.
       * @param current The model used for the current color.
       * @param palette The list model which holds a list of colors.
       * @param parent The parent widget.
       */
      ColorPicker(std::shared_ptr<ValueModel<QColor>> current,
        std::shared_ptr<ListModel<QColor>> palette, QWidget& parent);

      /** Returns the current color model. */
      const std::shared_ptr<ValueModel<QColor>>& get_current() const;

      /** Returns the list of colors. */
      const std::shared_ptr<ListModel<QColor>>& get_palette() const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      struct ColorPickerModel;
      std::shared_ptr<ColorPickerModel> m_model;
      std::shared_ptr<ListModel<QColor>> m_palette;
      OverlayPanel* m_panel;
      Slider2D* m_color_spectrum;
      Slider* m_alpha_slider;
      ColorCodePanel* m_color_code_panel;
      QColor m_last_color;
      int m_panel_horizontal_spacing;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_panel_style_connection;

      void on_current(const QColor& current);
      void on_panel_style();
  };
}

#endif
