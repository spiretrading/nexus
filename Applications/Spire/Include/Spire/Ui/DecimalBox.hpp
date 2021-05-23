#ifndef SPIRE_DECIMAL_BOX_HPP
#define SPIRE_DECIMAL_BOX_HPP
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/optional/optional.hpp>
#include <QHash>
#include <QRegularExpression>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/ScalarValueModel.hpp"
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
}

  /** Represents a widget for inputting decimal values. */
  class DecimalBox : public QWidget {
    public:

      /** The maximum precision of the Decimal type. */
      static constexpr auto PRECISION = 15;

      /** Represents the floating point type used by the DecimalBox. */
      using Decimal = boost::multiprecision::number<
        boost::multiprecision::cpp_dec_float<PRECISION>>;

      /** Type of model used by the DecimalBox. */
      using DecimalModel = ScalarValueModel<boost::optional<Decimal>>;

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
       * Constructs a DecimalBox with a LocalValueModel.
       * @param modifiers The initial keyboard modifier increments.
       * @param parent The parent widget.
       */
      explicit DecimalBox(QHash<Qt::KeyboardModifier, Decimal> modifiers,
        QWidget* parent = nullptr);

      /**
       * Constructs a DecimalBox with 6 decimal places and no trailing zeros.
       * @param model The model used for the current value.
       * @param modifiers The initial keyboard modifier increments.
       * @param parent The parent widget.
       */
      DecimalBox(std::shared_ptr<DecimalModel> model,
        QHash<Qt::KeyboardModifier, Decimal> modifiers,
        QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<DecimalModel>& get_model() const;

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
      void keyPressEvent(QKeyEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void wheelEvent(QWheelEvent* event) override;
      bool nativeEvent(const QByteArray& eventType, void* message,
        long* result) override;

    private:
      struct DecimalToTextModel;
      mutable SubmitSignal m_submit_signal;
      mutable RejectSignal m_reject_signal;
      std::shared_ptr<DecimalModel> m_model;
      std::shared_ptr<DecimalToTextModel> m_adaptor_model;
      boost::optional<Decimal> m_submission;
      QHash<Qt::KeyboardModifier, Decimal> m_modifiers;
      TextBox* m_text_box;
      QRegExp m_validator;
      Button* m_up_button;
      Button* m_down_button;
      Qt::Orientation m_mouse_wheel_orientation;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_submit_connection;
      boost::signals2::scoped_connection m_reject_connection;

      void decrement();
      void increment();
      Decimal get_increment() const;
      void step_by(const Decimal& value);
      void update_button_positions();
      void on_current(const boost::optional<Decimal>& current);
      void on_submit(const QString& submission);
      void on_reject(const QString& value);
      void on_style();
  };
}

#endif
