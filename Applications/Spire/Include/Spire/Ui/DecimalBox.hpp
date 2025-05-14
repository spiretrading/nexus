#ifndef SPIRE_DECIMAL_BOX_HPP
#define SPIRE_DECIMAL_BOX_HPP
#include <boost/optional/optional.hpp>
#include <QHash>
#include "Spire/Spire/Decimal.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** The number of leading zeros added as padding to a number. */
  using LeadingZeros = BasicProperty<int, struct LeadingZerosTag>;

  /**
   * The number of trailing zeros added as padding to the fractional part of a
   * number.
   */
  using TrailingZeros = BasicProperty<int, struct TrailingZerosTag>;

  /** The current value is positive. */
  using IsPositive = StateSelector<void, struct IsPositiveTag>;

  /** The current value is negative. */
  using IsNegative = StateSelector<void, struct IsNegativeTag>;

  /** The current value has increased. */
  using Uptick = StateSelector<void, struct UptickTag>;

  /** The current value has decreased. */
  using Downtick = StateSelector<void, struct DowntickTag>;
}

  /** Represents a widget for inputting decimal values. */
  class DecimalBox : public QWidget {
    public:

      /**
       * Signals that submission value has changed.
       * @param value The submission value.
       */
      using SubmitSignal = Signal<void (const boost::optional<Decimal>& value)>;

      /**
       * Signals that the current value was rejected as a submission.
       * @param value The value that was rejected.
       */
      using RejectSignal = Signal<void (const boost::optional<Decimal>& value)>;

      /**
       * Tests if a value is accepted by a DecimalBox, either because it is
       * valid, or because additional digits appended to the current value will
       * make it valid.
       * @param value The value to validate.
       * @param min The minimum allowable value.
       * @param max The maximum allowable value.
       * @return A value indicating the validation state.
       */
      static QValidator::State validate(const Decimal& value,
        const boost::optional<Decimal>& min,
        const boost::optional<Decimal>& max);

      /**
       * Constructs a DecimalBox with a LocalValueModel and
       * increments/decrements by 1.0.
       * @param parent The parent widget.
       */
      explicit DecimalBox(QWidget* parent = nullptr);

      /**
       * Constructs a DecimalBox with a LocalValueModel.
       * @param modifiers The initial keyboard modifier increments.
       * @param parent The parent widget.
       */
      explicit DecimalBox(QHash<Qt::KeyboardModifier, Decimal> modifiers,
        QWidget* parent = nullptr);

      /**
       * Constructs a DecimalBox with 6 decimal places and no trailing zeros
       * with an increment determined by the model's increment.
       * @param current The model used for the current value.
       * @param parent The parent widget.
       */
      explicit DecimalBox(std::shared_ptr<OptionalDecimalModel> current,
        QWidget* parent = nullptr);

      /**
       * Constructs a DecimalBox with 6 decimal places and no trailing zeros.
       * @param current The model used for the current value.
       * @param modifiers The initial keyboard modifier increments.
       * @param parent The parent widget.
       */
      DecimalBox(std::shared_ptr<OptionalDecimalModel> current,
        QHash<Qt::KeyboardModifier, Decimal> modifiers,
        QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<OptionalDecimalModel>& get_current() const;

      /** Returns the text displayed. */
      std::shared_ptr<const TextModel> get_text() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& value);

      /** Returns <code>true</code> iff this box is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the DecimalBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the value submission signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      /** Connects a slot to the RejectSignal. */
      boost::signals2::connection connect_reject_signal(
        const RejectSignal::slot_type& slot) const;

    protected:
      void changeEvent(QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void showEvent(QShowEvent* event) override;
      void timerEvent(QTimerEvent* event) override;
      void wheelEvent(QWheelEvent* event) override;

    private:
      enum class TickIndicator {
        NONE,
        UP,
        DOWN
      };
      enum class SignIndicator {
        NONE,
        POSITIVE,
        NEGATIVE
      };
      enum class ButtonType {
        NONE,
        UP,
        DOWN
      };
      struct DecimalToTextModel;
      struct EditableData {
        mutable SubmitSignal m_submit_signal;
        mutable RejectSignal m_reject_signal;
        boost::optional<Decimal> m_submission;
        Box* m_up_button;
        Box* m_down_button;
        boost::optional<FocusObserver> m_focus_observer;
        int m_repeat_delay_timer_id;
        int m_repeat_interval_timer_id;
        ButtonType m_pressed_button_type;
      };
      std::shared_ptr<OptionalDecimalModel> m_current;
      std::shared_ptr<DecimalToTextModel> m_adaptor_model;
      QHash<Qt::KeyboardModifier, Decimal> m_modifiers;
      TextBox m_text_box;
      boost::optional<Decimal> m_last_current;
      TickIndicator m_tick;
      SignIndicator m_sign;
      std::unique_ptr<EditableData> m_data;
      boost::signals2::scoped_connection m_style_connection;
      boost::signals2::scoped_connection m_current_connection;

      void initialize_editable_data() const;
      void decrement();
      void increment();
      Decimal get_increment() const;
      void reset();
      void step_by(const Decimal& value);
      void update_button_positions();
      void on_current(const boost::optional<Decimal>& current);
      void on_focus(const FocusObserver::State& state);
      void on_submit(const QString& submission);
      void on_reject(const QString& value);
      void on_style();
  };
}

#endif
