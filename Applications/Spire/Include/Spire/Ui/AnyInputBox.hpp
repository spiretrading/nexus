#ifndef SPIRE_ANY_INPUT_BOX_HPP
#define SPIRE_ANY_INPUT_BOX_HPP
#include <QWidget>
#include "Spire/Spire/AnyRef.hpp"
#include "Spire/Spire/AnyValueModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Details {
  template<class AlwaysVoid, template<class...> class Op, class... Args>
  struct detector {
    using value_t = std::false_type;
  };

  template<template<class...> class Op, class... Args>
  struct detector<std::void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
  };

  template<template<class...> class Op, class... Args>
  using is_detected = typename detector<void, Op, Args...>::value_t;

  template<typename T>
  using get_submission_type = decltype(std::declval<T>().get_submission());

  template<typename T>
  constexpr auto has_get_submission_v =
    is_detected<get_submission_type, T>::value;

  template<typename T>
  using get_highlight_type = decltype(std::declval<T>().get_highlight());

  template<typename T>
  constexpr auto has_get_highlight_v =
    is_detected<get_highlight_type, T>::value;

  template<typename T>
  using set_placeholder_type =
    decltype(std::declval<T>().set_placeholder(std::declval<QString>()));

  template<typename T>
  constexpr auto has_set_placeholder_v =
    is_detected<set_placeholder_type, T>::value;

  template<typename T>
  using is_read_only_type =
    decltype(std::declval<T>().is_read_only());

  template<typename T>
  constexpr auto has_is_read_only_v = is_detected<is_read_only_type, T>::value;

  template<typename T>
  using set_read_only_type =
    decltype(std::declval<T>().set_read_only(std::declval<bool>()));

  template<typename T>
  constexpr auto has_set_read_only_v =
    is_detected<set_read_only_type, T>::value;

  template<typename T>
  using connect_reject_signal_type =
    decltype(std::declval<T>().connect_reject_signal(
      std::declval<typename T::RejectSignal::slot_type>()));

  template<typename T>
  constexpr auto has_reject_signal_v =
    is_detected<connect_reject_signal_type, T>::value;
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

      /** Returns the highlight model. */
      const std::shared_ptr<HighlightModel>& get_highlight() const;

      /** Returns the last submission. */
      const AnyRef& get_submission() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& placeholder);

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
        virtual const std::shared_ptr<HighlightModel>&
          get_highlight() const = 0;
        virtual const AnyRef& get_submission() = 0;
        virtual void set_placeholder(const QString& placeholder) = 0;
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
        std::shared_ptr<HighlightModel> m_highlight;
        AnyRef m_submission;

        WrapperInputBox(T& input_box);
        const std::shared_ptr<AnyValueModel>& get_current() const override;
        const std::shared_ptr<HighlightModel>& get_highlight() const override;
        const AnyRef& get_submission() override;
        void set_placeholder(const QString& placeholder) override;
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
    setFocusProxy(&input_box);
    enclose(*this, input_box);
  }

  template<typename T>
  AnyInputBox::WrapperInputBox<T>::WrapperInputBox(T& input_box)
      : m_input_box(&input_box),
        m_current(std::make_shared<AnyValueModel>(m_input_box->get_current())) {
    if constexpr(!Details::has_get_highlight_v<T>) {
      m_highlight = std::make_shared<LocalValueModel<Highlight>>();
    }
  }

  template<typename T>
  const std::shared_ptr<AnyValueModel>&
      AnyInputBox::WrapperInputBox<T>::get_current() const {
    return m_current;
  }

  template<typename T>
  const std::shared_ptr<HighlightModel>&
      AnyInputBox::WrapperInputBox<T>::get_highlight() const {
    if constexpr(Details::has_get_highlight_v<T>) {
      return m_input_box->get_highlight();
    } else {
      return m_highlight;
    }
  }

  template<typename T>
  const AnyRef& AnyInputBox::WrapperInputBox<T>::get_submission() {
    if constexpr(Details::has_get_submission_v<T>) {
      m_submission = m_input_box->get_submission();
    }
    return m_submission;
  }

  template<typename T>
  void AnyInputBox::WrapperInputBox<T>::set_placeholder(
      const QString& placeholder) {
    if constexpr(Details::has_set_placeholder_v<T>) {
      m_input_box->set_placeholder(placeholder);
    }
  }

  template<typename T>
  bool AnyInputBox::WrapperInputBox<T>::is_read_only() const {
    if constexpr(Details::has_is_read_only_v<T>) {
      return m_input_box->is_read_only();
    } else {
      return false;
    }
  }

  template<typename T>
  void AnyInputBox::WrapperInputBox<T>::set_read_only(bool read_only) {
    if constexpr(Details::has_set_read_only_v<T>) {
      m_input_box->set_read_only(read_only);
    }
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
    } else {
      return {};
    }
  }
}

#endif
