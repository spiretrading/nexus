#ifndef SPIRE_ENUM_BOX_HPP
#define SPIRE_ENUM_BOX_HPP
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ListIndexValueModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Displays a single value among a closed set of homogenous values within a
   * DropDownBox.
   * @param <T> The type of enumerated values to display.
   */
  template<typename T>
  class EnumBox : public QWidget {
    public:

      /** The type of enumerated values to display. */
      using Type = T;

      /**
       * The type of model representing the current value.
       */
      using CurrentModel = ValueModel<Type>;

      /**
       * Signals that the value was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const Type& submission)>;

      /**
       * The type of function used to build a QWidget representing a selectable
       * value.
       * @param value The value to display.
       * @return The QWidget that shall be used to display the </i>value</i>.
       */
      using ViewBuilder = std::function<QWidget* (const Type& value)>;

      /**
       * The default view builder which uses a label styled TextBox to display
       * the text representation of its value.
       */
      static QWidget* default_view_builder(const Type& value);

      /** Stores the various settings used to build an EnumBox. */
      struct Settings {

        /** The list of values that can be selected. */
        std::shared_ptr<ListModel<Type>> m_cases;

        /** The current value's model. */
        std::shared_ptr<CurrentModel> m_current;

        /** The ViewBuilder to use. */
        ViewBuilder m_view_builder;

        /** Constructs Settings using default values. */
        Settings();
      };

      /**
       * Constructs an EnumBox.
       * @param settings The settings used by the EnumBox.
       * @param parent The parent widget.
       */
      explicit EnumBox(Settings settings, QWidget* parent = nullptr);

      /** Returns the model representing the current value. */
      const std::shared_ptr<CurrentModel>& get_current() const;

      /** Returns <code>true</code> iff this EnumBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only <code>true</code> iff the EnumBox should be
       *        read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const typename SubmitSignal::slot_type& slot) const;

    private:
      std::shared_ptr<CurrentModel> m_current;
      DropDownBox* m_drop_down_box;
  };

  template<typename T>
  QWidget* EnumBox<T>::default_view_builder(const Type& value) {
    return make_label(to_text(value));
  }

  template<typename T>
  EnumBox<T>::Settings::Settings()
    : m_view_builder(&default_view_builder) {}

  template<typename T>
  EnumBox<T>::EnumBox(Settings settings, QWidget* parent)
      : m_current(std::move(settings.m_current)) {
    if(!settings.m_cases) {
      auto model = std::make_shared<ArrayListModel<Type>>();
      model->push(m_current->get());
      settings.m_cases = std::move(model);
    } if(!m_current) {
      m_current =
        std::make_shared<LocalValueModel<Type>>(settings.m_cases->get(0));
    }
    m_drop_down_box = new DropDownBox(settings.m_cases,
      std::make_shared<ListIndexValueModel<Type>>(settings.m_cases, m_current),
      [view_builder = settings.m_view_builder] (
          const std::shared_ptr<ListModel<Type>>& model, auto index) {
        return view_builder(model->get(index));
      });
    enclose(*this, *m_drop_down_box);
    Styles::proxy_style(*this, *m_drop_down_box);
    setFocusProxy(m_drop_down_box);
  }

  template<typename T>
  const std::shared_ptr<typename EnumBox<T>::CurrentModel>&
      EnumBox<T>::get_current() const {
    return m_current;
  }

  template<typename T>
  bool EnumBox<T>::is_read_only() const {
    return m_drop_down_box->is_read_only();
  }

  template<typename T>
  void EnumBox<T>::set_read_only(bool is_read_only) {
    m_drop_down_box->set_read_only(is_read_only);
  }

  template<typename T>
  boost::signals2::connection EnumBox<T>::connect_submit_signal(
      const typename SubmitSignal::slot_type& slot) const {
    return m_drop_down_box->connect_submit_signal([=] (const auto& submission) {
      slot(std::any_cast<const Type&>(submission));
    });
  }
}

#endif
