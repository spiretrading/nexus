#ifndef SPIRE_SWITCH_BUTTON_HPP
#define SPIRE_SWITCH_BUTTON_HPP
#include <boost/optional/optional.hpp>
#include "Spire/Styles/PeriodicEvaluator.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/FocusObserver.hpp"

namespace Spire {
  class Box;

  /** Displays a toggle switch that slides between on and off states. */
  class SwitchButton : public QWidget {
    public:

      /**
       * Signals that the button was clicked.
       * @param checked <code>true</code> iff the SwitchButton is in the on
       *        state.
       */
      using SubmitSignal = Signal<void (bool checked)>;

      /**
       * Constructs a SwitchButton using a local model initialized to
       * <code>false</code>.
       * @param parent The parent widget.
       */
      explicit SwitchButton(QWidget* parent = nullptr);

      /**
       * Constructs a SwitchButton.
       * @param current The current value model.
       * @param parent The parent widget.
       */
      explicit SwitchButton(
        std::shared_ptr<BooleanModel> current, QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<BooleanModel>& get_current() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      void changeEvent(QEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<BooleanModel> m_current;
      Box* m_track;
      Box* m_switch;
      bool m_is_layout_done;
      boost::optional<Styles::PeriodicEvaluator<int>> m_switch_pos_evaluator;
      boost::optional<Styles::PeriodicEvaluator<QColor>>
        m_track_color_evaluator;
      boost::optional<FocusObserver> m_focus_observer;
      boost::signals2::scoped_connection m_connection;

      bool is_focus_visible() const;
      int get_switch_position(bool checked) const;
      void animate_switch_position(bool checked);
      void animate_track_color(const QColor& target);
      void on_click();
      void on_current(bool current);
      void on_focus(FocusObserver::State state);
  };
}

#endif
