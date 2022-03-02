#ifndef SPIRE_DECIMAL_BOX_ADAPTOR_HPP
#define SPIRE_DECIMAL_BOX_ADAPTOR_HPP
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a wrapper over a DecimalBox for use with a scalar type.
   * @param <T> The type of scalar to display.
   */
  template<typename T>
  class DecimalBoxAdaptor : public QWidget {
    public:

      /** The type of scalar to display. */
      using Type = T;

      /**
       * Signals a submission.
       * @param value The submitted value.
       */
      using SubmitSignal = Signal<void (boost::optional<Type> value)>;

      /**
       * Signals that the current value was rejected as a submission.
       * @param value The value that was rejected.
       */
      using RejectSignal = Signal<void (boost::optional<Type> value)>;

      /**
       * Constructs a DecimalBoxAdaptor with a LocalValueModel with modifiers
       * set to the model's increment.
       * @param parent The parent widget.
       */
      explicit DecimalBoxAdaptor(QWidget* parent = nullptr);

      /**
       * Constructs a DecimalBoxAdaptor with a LocalValueModel.
       * @param modifiers The keyboard modifier increments.
       * @param parent The parent widget.
       */
      explicit DecimalBoxAdaptor(
        QHash<Qt::KeyboardModifier, Type> modifiers, QWidget* parent = nullptr);

      /**
       * Constructs a DecimalBoxAdaptor with modifiers set to the model's
       * increment.
       * @param current The model used for the current value.
       * @param parent The parent widget.
       */
      DecimalBoxAdaptor(
        std::shared_ptr<ScalarValueModel<boost::optional<Type>>> current,
        QWidget* parent = nullptr);

      /**
       * Constructs a DecimalBoxAdaptor.
       * @param current The model used for the current value.
       * @param modifiers The keyboard modifier increments.
       * @param parent The parent widget.
       */
      DecimalBoxAdaptor(
        std::shared_ptr<ScalarValueModel<boost::optional<Type>>> current,
        QHash<Qt::KeyboardModifier, Type> modifiers, QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<ScalarValueModel<boost::optional<Type>>>&
        get_current() const;

      /** Returns the text displayed. */
      std::shared_ptr<const TextModel> get_text() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& value);

      /** Returns <code>true</code> iff this box is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the MoneyBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the value submission signal. */
      boost::signals2::connection connect_submit_signal(
        const typename SubmitSignal::slot_type& slot) const;

      /** Connects a slot to the RejectSignal. */
      boost::signals2::connection connect_reject_signal(
        const typename RejectSignal::slot_type& slot) const;

    protected:

      /**
       * Constructs a DecimalBoxAdaptor with a custom adaptor model.
       * @param current The model used for the current value.
       * @param adaptor_model The model adapting from the scalar to the Decimal.
       * @param parent The parent widget.
       */
      DecimalBoxAdaptor(
        std::shared_ptr<ScalarValueModel<boost::optional<Type>>> current,
        std::shared_ptr<ToDecimalModel<Type>> adaptor_model,
        QWidget* parent = nullptr);

      /**
       * Constructs a DecimalBoxAdaptor with a custom adaptor model.
       * @param current The model used for the current value.
       * @param adaptor_model The model adapting from the scalar to the Decimal.
       * @param modifiers The keyboard modifier increments.
       * @param parent The parent widget.
       */
      DecimalBoxAdaptor(
        std::shared_ptr<ScalarValueModel<boost::optional<Type>>> current,
        std::shared_ptr<ToDecimalModel<Type>> adaptor_model,
        QHash<Qt::KeyboardModifier, Type> modifiers, QWidget* parent = nullptr);

      /** Returns the DecimalBox used to display the scalar. */
      DecimalBox& get_decimal_box();

    private:
      mutable SubmitSignal m_submit_signal;
      mutable RejectSignal m_reject_signal;
      std::shared_ptr<ScalarValueModel<boost::optional<Type>>> m_current;
      std::shared_ptr<ToDecimalModel<Type>> m_adaptor_model;
      boost::optional<Type> m_submission;
      DecimalBox* m_decimal_box;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_submit_connection;
      boost::signals2::scoped_connection m_reject_connection;

      static auto make_modifiers(
        const ScalarValueModel<boost::optional<Type>>& model);
      void on_submit(const boost::optional<Decimal>& submission);
      void on_reject(const boost::optional<Decimal>& value);
  };

  template<typename T>
  DecimalBoxAdaptor<T>::DecimalBoxAdaptor(QWidget* parent)
    : DecimalBoxAdaptor(
        std::make_shared<LocalScalarValueModel<boost::optional<Type>>>(),
        parent) {}

  template<typename T>
  DecimalBoxAdaptor<T>::DecimalBoxAdaptor(
    QHash<Qt::KeyboardModifier, Type> modifiers, QWidget* parent)
    : DecimalBoxAdaptor(
        std::make_shared<LocalScalarValueModel<boost::optional<Type>>>(),
        std::move(modifiers), parent) {}

  template<typename T>
  DecimalBoxAdaptor<T>::DecimalBoxAdaptor(
    std::shared_ptr<ScalarValueModel<boost::optional<Type>>> current,
    QWidget* parent)
    : DecimalBoxAdaptor(
        current, std::make_shared<ToDecimalModel<Type>>(current), parent) {}

  template<typename T>
  DecimalBoxAdaptor<T>::DecimalBoxAdaptor(
    std::shared_ptr<ScalarValueModel<boost::optional<Type>>> current,
    QHash<Qt::KeyboardModifier, Type> modifiers, QWidget* parent)
    : DecimalBoxAdaptor(current,
        std::make_shared<ToDecimalModel<Type>>(current), std::move(modifiers),
        parent) {}

  template<typename T>
  const std::shared_ptr<ScalarValueModel<
      boost::optional<typename DecimalBoxAdaptor<T>::Type>>>&
        DecimalBoxAdaptor<T>::get_current() const {
    return m_current;
  }

  template<typename T>
  std::shared_ptr<const TextModel> DecimalBoxAdaptor<T>::get_text() const {
    return m_decimal_box->get_text();
  }

  template<typename T>
  void DecimalBoxAdaptor<T>::set_placeholder(const QString& value) {
    m_decimal_box->set_placeholder(value);
  }

  template<typename T>
  bool DecimalBoxAdaptor<T>::is_read_only() const {
    return m_decimal_box->is_read_only();
  }

  template<typename T>
  void DecimalBoxAdaptor<T>::set_read_only(bool is_read_only) {
    m_decimal_box->set_read_only(is_read_only);
  }

  template<typename T>
  boost::signals2::connection DecimalBoxAdaptor<T>::connect_submit_signal(
      const typename SubmitSignal::slot_type& slot) const {
    return m_submit_signal.connect(slot);
  }

  template<typename T>
  boost::signals2::connection DecimalBoxAdaptor<T>::connect_reject_signal(
      const typename RejectSignal::slot_type& slot) const {
    return m_reject_signal.connect(slot);
  }

  template<typename T>
  DecimalBoxAdaptor<T>::DecimalBoxAdaptor(
    std::shared_ptr<ScalarValueModel<boost::optional<Type>>> current,
    std::shared_ptr<ToDecimalModel<Type>> adaptor_model, QWidget* parent)
    : DecimalBoxAdaptor(current, std::move(adaptor_model),
        make_modifiers(*current), parent) {}

  template<typename T>
  DecimalBoxAdaptor<T>::DecimalBoxAdaptor(
      std::shared_ptr<ScalarValueModel<boost::optional<Type>>> current,
      std::shared_ptr<ToDecimalModel<Type>> adaptor_model,
      QHash<Qt::KeyboardModifier, Type> modifiers, QWidget* parent)
      : QWidget(parent),
        m_current(std::move(current)),
        m_adaptor_model(std::move(adaptor_model)),
        m_submission(m_current->get()) {
    auto adapted_modifiers = QHash<Qt::KeyboardModifier, Decimal>();
    for(auto modifier = modifiers.begin();
        modifier != modifiers.end(); ++modifier) {
      adapted_modifiers.insert(modifier.key(), to_decimal(modifier.value()));
    }
    m_decimal_box =
      new DecimalBox(m_adaptor_model, std::move(adapted_modifiers), this);
    Styles::proxy_style(*this, *m_decimal_box);
    setFocusProxy(m_decimal_box);
    enclose(*this, *m_decimal_box);
    m_submit_connection = m_decimal_box->connect_submit_signal(
      [=] (const auto& submission) { on_submit(submission); });
    m_reject_connection = m_decimal_box->connect_reject_signal(
      [=] (const auto& value) { on_reject(value); });
  }

  template<typename T>
  DecimalBox& DecimalBoxAdaptor<T>::get_decimal_box() {
    return *m_decimal_box;
  }

  template<typename T>
  auto DecimalBoxAdaptor<T>::make_modifiers(
      const ScalarValueModel<boost::optional<Type>>& model) {
    auto modifiers = QHash<Qt::KeyboardModifier, Type>();
    modifiers[Qt::NoModifier] = model.get_increment();
    return modifiers;
  }

  template<typename T>
  void DecimalBoxAdaptor<T>::on_submit(
      const boost::optional<Decimal>& submission) {
    m_submission = m_current->get();
    m_submit_signal(m_submission);
  }

  template<typename T>
  void DecimalBoxAdaptor<T>::on_reject(const boost::optional<Decimal>& value) {
    m_reject_signal(from_decimal<Type>(value));
  }
}

#endif
