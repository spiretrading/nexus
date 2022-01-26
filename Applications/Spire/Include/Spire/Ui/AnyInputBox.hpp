#ifndef SPIRE_ANY_INPUT_BOX_HPP
#define SPIRE_ANY_INPUT_BOX_HPP
#include <QHBoxLayout>
#include <QWidget>
#include "Spire/Spire/AnyRef.hpp"
#include "Spire/Spire/AnyValueModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Details {
  template<typename T, typename = void>
  struct has_reject_signal : std::false_type {};

  template<typename T>
  struct has_reject_signal<T, typename T::RejectSignal> : std::true_type {};

  template<typename T>
  constexpr auto has_reject_signal_v = has_reject_signal<T>::value;

  template <class AlwaysVoid, template<class...> class Op, class... Args>
  struct detector {
    using value_t = std::false_type;
  };

  template <template<class...> class Op, class... Args>
  struct detector<std::void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
  };

  template <template<class...> class Op, class... Args>
  using is_detected = typename detector<void, Op, Args...>::value_t;

  template <typename T>
  using get_submission_type = decltype(std::declval<T>().get_submission());

  template <typename T>
  constexpr auto has_get_submission_v =
    is_detected<get_submission_type, T>::value;
}

  /** Encapsulates a generic input box. */
  class AnyInputBox : public QWidget {
    public:

      /**
       * Signals that the current value is being submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const AnyRef& submission)>;

      /**
       * Signals that the current value was rejected as a submission.
       * @param value The rejected value.
       */
      using RejectSignal = Signal<void (const AnyRef& value)>;

      /**
       * Constructs an AnyInputBox.
       * @param input_box The input box to encapsulate.
       * @param parent The parent widget.
       */
      template<typename T>
      explicit AnyInputBox(T& input_box, QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<AnyValueModel>& get_current() const;

      /** Returns the last submission. */
      const AnyRef& get_submission() const;

      /** Returns <code>true</code> iff this input box is read-only. */
      bool is_read_only() const;

      /** Sets whether this input box is read-only. */
      void set_read_only(bool read_only);

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      /** Connects a slot to the RejectedSignal. */
      boost::signals2::connection connect_reject_signal(
        const RejectSignal::slot_type& slot) const;

    private:
      struct AbstractInputBox {
        virtual ~AbstractInputBox() = default;
        virtual const std::shared_ptr<AnyValueModel>& get_current() const = 0;
        virtual const AnyRef& get_submission() = 0;
        virtual bool is_read_only() const = 0;
        virtual void set_read_only(bool read_only) = 0;
        virtual boost::signals2::connection connect_submit_signal(
          const SubmitSignal::slot_type& slot) const = 0;
        virtual boost::signals2::connection connect_reject_signal(
          const RejectSignal::slot_type& slot) const = 0;
      };
      template<typename T>
      struct WrapperInputBox final : AbstractInputBox {
        T* m_input_box;
        std::shared_ptr<AnyValueModel> m_current;
        AnyRef m_submission;
        boost::signals2::scoped_connection m_submit_connection;
        boost::signals2::scoped_connection m_reject_connection;

        WrapperInputBox(T& input_box);
        const std::shared_ptr<AnyValueModel>& get_current() const override;
        const AnyRef& get_submission() override;
        bool is_read_only() const override;
        void set_read_only(bool read_only) override;
        boost::signals2::connection connect_submit_signal(
          const SubmitSignal::slot_type& slot) const override;
        boost::signals2::connection connect_reject_signal(
          const RejectSignal::slot_type& slot) const override;
      };
      std::unique_ptr<AbstractInputBox> m_input_box;
  };

  template<typename T>
  AnyInputBox::AnyInputBox(T& input_box, QWidget* parent)
      : QWidget(parent),
        m_input_box(std::make_unique<WrapperInputBox<T>>(input_box)) {
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins({});
    layout->addWidget(&input_box);
  }

  template<typename T>
  AnyInputBox::WrapperInputBox<T>::WrapperInputBox(T& input_box)
    : m_input_box(&input_box),
      m_current(std::make_shared<AnyValueModel>(m_input_box->get_current())) {}

  template<typename T>
  const std::shared_ptr<AnyValueModel>&
      AnyInputBox::WrapperInputBox<T>::get_current() const {
    return m_current;
  }

  template<typename T>
  const AnyRef& AnyInputBox::WrapperInputBox<T>::get_submission() {
    if constexpr(Details::has_get_submission_v<T>) {
      m_submission = m_input_box->get_submission();
    }
    return m_submission;
  }

  template<typename T>
  bool AnyInputBox::WrapperInputBox<T>::is_read_only() const {
    return m_input_box->is_read_only();
  }

  template<typename T>
  void AnyInputBox::WrapperInputBox<T>::set_read_only(bool read_only) {
    m_input_box->set_read_only(read_only);
  }

  template<typename T>
  boost::signals2::connection AnyInputBox::WrapperInputBox<T>::
      connect_submit_signal(const SubmitSignal::slot_type& slot) const {
    return m_input_box->connect_submit_signal(slot);
  }

  template<typename T>
  boost::signals2::connection AnyInputBox::WrapperInputBox<T>::
      connect_reject_signal(const RejectSignal::slot_type& slot) const {
    if constexpr(Details::has_reject_signal_v<T>) {
      return m_input_box->connect_reject_signal(slot);
    }
    return {};
  }
}

#endif
