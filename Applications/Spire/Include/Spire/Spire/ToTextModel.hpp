#ifndef SPIRE_TO_TEXT_MODEL_HPP
#define SPIRE_TO_TEXT_MODEL_HPP
#include <functional>
#include <boost/optional/optional.hpp>
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  /**
   * Implements a model that wraps a reference model and converts its values to
   * and from their string representations.
   * @param <T> The type of the reference model.
   */
  template<typename T>
  class ToTextModel : public ValueModel<QString> {
    public:

      /** The type used by the reference model. */
      using Source = T;

      /**
       * Function for converting a value to a QString.
       * @param value The value to convert to a string.
       * @returns The string representation of the given value.
       */
      using ToString = std::function<QString (const Source& value)>;

      /**
       * Function for converting a QString to a value.
       * @param value The string representation.
       * @returns An initialized optional iff the conversion was successful.
       */
      using FromString =
        std::function<boost::optional<Source> (const QString& value)>;

      /**
       * Constructs a ToTextModel with default conversion functions.
       * @param model The reference model.
       */
      explicit ToTextModel(std::shared_ptr<ValueModel<Source>> model);

      /**
       * Constructs a ToTextModel.
       * @param model The reference model.
       * @param to_string The value to QString conversion function.
       */
      ToTextModel(
        std::shared_ptr<ValueModel<Source>> model, ToString to_string);

      /**
       * Constructs a ToTextModel.
       * @param model The reference model.
       * @param from_string The QString to value conversion function.
       */
      ToTextModel(
        std::shared_ptr<ValueModel<Source>> model, FromString from_string);

      /**
       * Constructs a ToTextModel.
       * @param model The reference model.
       * @param to_string The value to QString conversion function.
       * @param from_string The QString to value conversion function.
       */
      ToTextModel(
        std::shared_ptr<ValueModel<Source>> model, ToString to_string,
        FromString from_string);

      QValidator::State get_state() const override;

      const QString& get() const override;

      QValidator::State set(const QString& value) override;

      boost::signals2::connection connect_current_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      mutable UpdateSignal m_current_signal;
      std::shared_ptr<ValueModel<Source>> m_model;
      ToString m_to_string;
      FromString m_from_string;
      QString m_current;
      boost::signals2::scoped_connection m_current_connection;

      void on_current(const Source& current);
  };

  template<typename T>
  ToTextModel<T>::ToTextModel(std::shared_ptr<ValueModel<Source>> model)
    : ToTextModel(std::move(model), [] (const Source& value) {
          return CustomVariantItemDelegate().displayText(
            QVariant::fromValue(value));
        }, &Spire::from_string<Source>) {}

  template<typename T>
  ToTextModel<T>::ToTextModel(
    std::shared_ptr<ValueModel<Source>> model, ToString to_string)
    : ToTextModel(
        std::move(model), std::move(to_string), &Spire::from_string<Source>) {}

  template<typename T>
  ToTextModel<T>::ToTextModel(
    std::shared_ptr<ValueModel<Source>> model, FromString from_string)
    : ToTextModel(std::move(model),
        [] (const Source& value) {
          return CustomVariantItemDelegate().displayText(
            QVariant::fromValue(value));
        }, from_string) {}

  template<typename T>
  ToTextModel<T>::ToTextModel(std::shared_ptr<ValueModel<Source>> model,
    ToString to_string, FromString from_string)
    : m_model(std::move(model)),
      m_to_string(std::move(to_string)),
      m_from_string(std::move(from_string)),
      m_current(m_to_string(m_model->get())),
      m_current_connection(m_model->connect_current_signal(
        [=] (const auto& current) { on_current(current); })) {}

  template<typename T>
  QValidator::State ToTextModel<T>::get_state() const {
    return m_model->get_state();
  }

  template<typename T>
  const QString& ToTextModel<T>::get() const {
    return m_current;
  }

  template<typename T>
  QValidator::State ToTextModel<T>::set(const QString& value) {
    if(auto current = m_from_string(value)) {
      return m_model->set(*current);
    }
    return QValidator::Invalid;
  }

  template<typename T>
  boost::signals2::connection ToTextModel<T>::connect_current_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return m_current_signal.connect(slot);
  }

  template<typename T>
  void ToTextModel<T>::on_current(const Source& current) {
    m_current = m_to_string(current);
    m_current_signal(m_current);
  }
}

#endif
