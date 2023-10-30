#ifndef SPIRE_HIGHLIGHT_BOX_HPP
#define SPIRE_HIGHLIGHT_BOX_HPP
#include <any>
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/HighlightSwatch.hpp"
#include "Spire/Ui/PressObserver.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  using HighlightColor = HighlightSwatch::Highlight;

  /** A ValueModel over a HighlightColor. */
  using HighlightColorModel = ValueModel<HighlightColor>;

  /** A LocalValueModel over a HighlightColor. */
  using LocalHighlightColorModel = LocalValueModel<HighlightColor>;

  /**
   * Represents a highlight which is a combination of background color and
   * text color.
   */
  class HighlightBox : public QWidget {
    public:

      /**
       * Signals that the current highlight is being submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const HighlightColor& submission)>;

      /**
       * Constructs a HighlightBox with a default local model.
       * @param parent The parent widget.
       */
      explicit HighlightBox(QWidget* parent = nullptr);

      /**
       * Constructs a HighlightBox.
       * @param current The model used for the current color.
       * @param parent The parent widget.
       */
      explicit HighlightBox(std::shared_ptr<HighlightColorModel> current,
        QWidget* parent = nullptr);

      /** Returns the current highlight model. */
      const std::shared_ptr<HighlightColorModel>& get_current() const;

      /** Returns <code>true</code> iff this HighlightBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the HighlightBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the value submission signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<HighlightColorModel> m_current;
      HighlightColor m_submission;
      HighlightPicker* m_highlight_picker;
      QWidget* m_highlight_picker_panel;
      Box* m_input_box;
      bool m_is_read_only;
      bool m_is_modified;
      FocusObserver m_focus_observer;
      PressObserver m_press_observer;
      boost::signals2::scoped_connection m_current_connection;

      void show_highlight_picker();
      void submit();
      void update_label_color(const HighlightColor& highlight);
      void on_palette_submit(const std::any& submission);
      void on_current(const HighlightColor& current);
      void on_focus(FocusObserver::State state);
      void on_press_end(PressObserver::Reason reason);
  };
}

#endif
