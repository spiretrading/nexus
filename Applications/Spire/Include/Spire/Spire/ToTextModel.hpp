#ifndef SPIRE_TO_TEXT_MODEL_HPP
#define SPIRE_TO_TEXT_MODEL_HPP
#include <functional>
#include <boost/optional/optional.hpp>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  /**
   * Implements a model that wraps a reference model and converts its values to
   * and from their string representations.
   * @param <T> The type of value being converted to text.
   */
  template<typename T>
  class ToTextModel : public ValueModel<QString> {
    public:

      /** The type of value being converted to text. */
      using Source = T;

      /**
       * Function for converting a value to its text representation.
       * @param value The value to convert to text.
       * @returns The text representation of the given value.
       */
      using ToText = std::function<QString (const Source& value)>;

      /**
       * Function for converting text back to its native value.
       * @param value The text representation.
       * @returns An initialized optional iff the conversion was successful.
       */
      using FromText =
        std::function<boost::optional<Source> (const QString& value)>;

      /**
       * Constructs a ToTextModel with default conversion functions.
       * @param source The model to convert to text.
       */
      explicit ToTextModel(std::shared_ptr<ValueModel<Source>> source);

      /**
       * Constructs a ToTextModel.
       * @param source The model to convert to text.
       * @param to_text The value to text conversion function.
       */
      ToTextModel(std::shared_ptr<ValueModel<Source>> source, ToText to_text);

      /**
       * Constructs a ToTextModel.
       * @param source The model to convert to text.
       * @param from_text The text to value conversion function.
       */
      ToTextModel(
        std::shared_ptr<ValueModel<Source>> source, FromText from_text);

      /**
       * Constructs a ToTextModel.
       * @param source The model to convert to text.
       * @param to_text The value to text conversion function.
       * @param from_text The text to value conversion function.
       */
      ToTextModel(std::shared_ptr<ValueModel<Source>> source, ToText to_text,
        FromText from_text);

      QValidator::State get_state() const override;

      const QString& get() const override;

      QValidator::State test(const QString& value) const override;

      QValidator::State set(const QString& value) override;

      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<ValueModel<Source>> m_source;
      ToText m_to_text;
      FromText m_from_text;
      LocalValueModel<QString> m_model;
      boost::signals2::scoped_connection m_update_connection;

      void on_update(const Source& value);
  };

  /**
   * Constructs a ToTextModel with default conversion functions.
   * @param source The model to convert to text.
   */
  template<typename T>
  auto make_to_text_model(std::shared_ptr<ValueModel<T>> source) {
    return std::make_shared<ToTextModel<T>>(std::move(source));
  }

  /**
   * Constructs a ToTextModel with default conversion functions.
   * @param source The model to convert to text.
   * @param to_text The function used to convert a value to its text
   *        representation.
   */
  template<typename T>
  auto make_to_text_model(std::shared_ptr<ValueModel<T>> source,
      typename ToTextModel<T>::ToText to_text) {
    return std::make_shared<ToTextModel<T>>(
      std::move(source), std::move(to_text));
  }

  template<typename T>
  ToTextModel<T>::ToTextModel(std::shared_ptr<ValueModel<Source>> source)
    : ToTextModel(std::move(source), [] (const Source& value) {
        return to_text(value);
      }, &Spire::from_text<Source>) {}

  template<typename T>
  ToTextModel<T>::ToTextModel(
    std::shared_ptr<ValueModel<Source>> source, ToText to_text)
    : ToTextModel(
        std::move(source), std::move(to_text), &Spire::from_text<Source>) {}

  template<typename T>
  ToTextModel<T>::ToTextModel(
    std::shared_ptr<ValueModel<Source>> source, FromText from_text)
    : ToTextModel(std::move(source), [] (const Source& value) {
        return to_text(value);
      }, &Spire::from_text<Source>, from_text) {}

  template<typename T>
  ToTextModel<T>::ToTextModel(std::shared_ptr<ValueModel<Source>> source,
    ToText to_text, FromText from_text)
    : m_source(std::move(source)),
      m_to_text(std::move(to_text)),
      m_from_text(std::move(from_text)),
      m_model(m_to_text(m_source->get())),
      m_update_connection(m_source->connect_update_signal(
        std::bind_front(&ToTextModel::on_update, this))) {}

  template<typename T>
  QValidator::State ToTextModel<T>::get_state() const {
    return m_source->get_state();
  }

  template<typename T>
  const QString& ToTextModel<T>::get() const {
    return m_model.get();
  }

  template<typename T>
  QValidator::State ToTextModel<T>::test(const QString& value) const {
    if(auto update = m_from_text(value)) {
      return m_source->test(*update);
    }
    return QValidator::Invalid;
  }

  template<typename T>
  QValidator::State ToTextModel<T>::set(const QString& value) {
    if(auto update = m_from_text(value)) {
      return m_source->set(*update);
    }
    return QValidator::Invalid;
  }

  template<typename T>
  boost::signals2::connection ToTextModel<T>::connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return m_model.connect_update_signal(slot);
  }

  template<typename T>
  void ToTextModel<T>::on_update(const Source& value) {
    m_model.set(m_to_text(value));
  }
}

#endif
