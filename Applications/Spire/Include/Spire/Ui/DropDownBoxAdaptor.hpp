#ifndef SPIRE_DROP_DOWN_BOX_ADAPTOR_HPP
#define SPIRE_DROP_DOWN_BOX_ADAPTOR_HPP
#include <QHBoxLayout>
#include <QWidget>
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a wrapper over a DropDownBox for use with a type.
   * @param <T> The type of value to display in the DropDownBox.
   */
  template<typename T>
  class DropDownBoxAdaptor : public QWidget {
    public:

      /** The type of the values used to display in the DropDownBox. */
      using Type = T;

      /**
       * Signals that the value was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const Type& submission)>;

      /**
       * Constructs a DropDownBoxAdaptor using a LocalValueModel.
       * @param list_model The model of values to display.
       * @param parent The parent widget.
       */
      DropDownBoxAdaptor(std::shared_ptr<ListModel> list_model,
        QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBoxAdaptor.
       * @param list_model The model of values to display.
       * @param current_model The current value's model.
       * @param parent The parent widget.
       */
      DropDownBoxAdaptor(std::shared_ptr<ListModel> list_model,
        std::shared_ptr<ValueModel<boost::optional<int>>> current_model,
        QWidget* parent = nullptr);

      /** Returns the list model. */
      const std::shared_ptr<ListModel>& get_list_model() const;

      /** Returns the current model. */
      const std::shared_ptr<ValueModel<boost::optional<int>>>&
        get_current_model() const;

      /** Returns <code>true</code> iff this DropDownBoxAdaptor is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the DropDownBoxAdaptor should be
       *        read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const typename SubmitSignal::slot_type& slot) const;

      private:
        mutable SubmitSignal m_submit_signal;
        std::shared_ptr<ListModel> m_list_model;
        std::shared_ptr<ValueModel<boost::optional<int>>> m_current_model;
        ListView* m_list_view;
        DropDownBox* m_drop_down_box;
        boost::signals2::scoped_connection m_current_connection;

        void on_current(const boost::optional<int>& current);
  };

  template<typename T>
  DropDownBoxAdaptor<T>::DropDownBoxAdaptor(
    std::shared_ptr<ListModel> list_model, QWidget* parent)
    : DropDownBoxAdaptor(std::move(list_model),
        std::make_shared<LocalValueModel<optional<int>>>(), parent) {}

  template<typename T>
  DropDownBoxAdaptor<T>::DropDownBoxAdaptor(
      std::shared_ptr<ListModel> list_model,
      std::shared_ptr<ValueModel<boost::optional<int>>> current_model,
      QWidget* parent)
      : QWidget(parent),
        m_list_model(std::move(list_model)),
        m_current_model(std::move(current_model)) {
    m_list_view = new ListView(m_list_model);
    m_list_view->get_current_model()->set_current(
      m_current_model->get_current());
    m_drop_down_box = new DropDownBox(*m_list_view);
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins({});
    layout->addWidget(m_drop_down_box);
    m_current_connection = m_current_model->connect_current_signal(
      [=] (const auto& current) { on_current(current); });
    m_drop_down_box->connect_submit_signal([=] (const auto& submission) {
      m_submit_signal(std::any_cast<Type>(submission));
    });
  }

  template<typename T>
  const std::shared_ptr<ListModel>&
      DropDownBoxAdaptor<T>::get_list_model() const {
    return m_list_model;
  }

  template<typename T>
  const std::shared_ptr<ValueModel<boost::optional<int>>>&
      DropDownBoxAdaptor<T>::get_current_model() const {
    return m_current_model;
  }

  template<typename T>
  bool DropDownBoxAdaptor<T>::is_read_only() const {
    return m_drop_down_box->is_read_only();
  }

  template<typename T>
  void DropDownBoxAdaptor<T>::set_read_only(bool is_read_only) {
    m_drop_down_box->set_read_only(is_read_only);
  }

  template<typename T>
  boost::signals2::connection DropDownBoxAdaptor<T>::connect_submit_signal(
      const typename SubmitSignal::slot_type& slot) const {
    return m_submit_signal.connect(slot);
  }

  template<typename T>
  void DropDownBoxAdaptor<T>::on_current(const boost::optional<int>& current) {
    m_list_view->get_current_model()->set_current(current);
  }
}

#endif
