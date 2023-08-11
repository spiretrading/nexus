#ifndef SPIRE_HIGHLIGHT_PICKER_HPP
#define SPIRE_HIGHLIGHT_PICKER_HPP
#include "Spire/Ui/HighlightSwatch.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a Label to preview a highlight. */
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
      explicit HighlightPicker(std::shared_ptr<ValueModel<Highlight>> current,
        QWidget& parent);

      /** Returns the current value model. */
      const std::shared_ptr<ValueModel<Highlight>>& get_current() const;

    protected:
      bool event(QEvent* event) override;

    private:
      std::shared_ptr<ValueModel<Highlight>> m_current;
      ListView* m_palette;
      OverlayPanel* m_panel;
      boost::signals2::scoped_connection m_current_connection;

      void on_current(const Highlight& highlight);
      void on_palette_current(boost::optional<int> current);
  };
}

#endif
