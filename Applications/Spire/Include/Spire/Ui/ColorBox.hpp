#ifndef SPIRE_COLOR_BOX_HPP
#define SPIRE_COLOR_BOX_HPP
#include <memory>
#include <QColor>
#include <QWidget>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/PressObserver.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** A ValueModel over a QColor. */
  using ColorModel = ValueModel<QColor>;

  /** A LocalValueModel over a QColor. */
  using LocalColorModel = LocalValueModel<QColor>;

  /**
   * Presents a color.
   */
  class ColorBox : public QWidget {
    public:

      /**
       * Signals that the current color is being submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const QColor& submission)>;

      /**
       * Constructs a ColorBox with a default local model.
       * @param parent The parent widget.
       */
      explicit ColorBox(QWidget* parent = nullptr);

      /**
       * Constructs a ColorBox.
       * @param current The model used for the current color.
       * @param parent The parent widget.
       */
      explicit ColorBox(std::shared_ptr<ColorModel> current,
        QWidget* parent = nullptr);

      /** Returns the current color model. */
      const std::shared_ptr<ColorModel>& get_current() const;

      /** Returns <code>true</code> iff this ColorBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the ColorBox should be read-only.
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
      std::shared_ptr<ColorModel> m_current;
      Box* m_color_display;
      Box* m_input_box;
      ColorPicker* m_color_picker;
      QWidget* m_color_picker_panel;
      QColor m_submission;
      bool m_is_read_only;
      bool m_is_modified;
      FocusObserver m_focus_observer;
      PressObserver m_press_observer;
      boost::signals2::scoped_connection m_current_connection;

      void show_color_picker();
      void submit();
      void on_current(const QColor& current);
      void on_focus(FocusObserver::State state);
      void on_press_end(PressObserver::Reason reason);
  };
}

#endif
