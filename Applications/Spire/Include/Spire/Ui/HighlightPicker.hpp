#ifndef SPIRE_HIGHLIGHT_PICKER_HPP
#define SPIRE_HIGHLIGHT_PICKER_HPP
#include "Spire/Ui/HighlightSwatch.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a panel where the user can select and custom highlight. */
  class HighlightPicker : public QWidget {
    public:

      using Highlight = HighlightSwatch::Highlight;

      /**
       * Constructs a HighlightPicker with a LocalValueModel.
       * @param parent The parent widget.
       */
      explicit HighlightPicker(QWidget& parent);

      /**
       * Constructs a HighlightPicker.
       * @param current The current value model.
       * @param parent The parent widget.
       */
      HighlightPicker(std::shared_ptr<ValueModel<Highlight>> current,
        QWidget& parent);

      /** Returns the current value model. */
      const std::shared_ptr<ValueModel<Highlight>>& get_current() const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      struct HighlightPickerModel;
      std::shared_ptr<HighlightPickerModel> m_model;
      ListView* m_palette;
      OverlayPanel* m_panel;
      ColorBox* m_background_color_box;
      ColorBox* m_text_color_box;
      boost::signals2::scoped_connection m_current_connection;

      bool handle_mouse_press(ColorBox& source, ColorBox& destination, const QMouseEvent& mouse_event);
      void on_palette_current(boost::optional<int> current);
  };
}

#endif
