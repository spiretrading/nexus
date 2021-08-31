#ifndef SPIRE_DROP_DOWN_BOX_ADAPTOR_HPP
#define SPIRE_DROP_DOWN_BOX_ADAPTOR_HPP
#include <QHBoxLayout>
#include <QWidget>
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/TextBox.hpp"
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
       * @param parent The parent widget.
       */
      explicit DropDownBoxAdaptor(QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBoxAdaptor using a LocalValueModel and
       * initial current value.
       * @param current The initial current value.
       * @param parent The parent widget.
       */
      explicit DropDownBoxAdaptor(Type current, QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBoxAdaptor.
       * @param parent The parent widget.
       */
      explicit DropDownBoxAdaptor(std::shared_ptr<ValueModel<Type>> model,
        QWidget* parent = nullptr);

      /** Returns the model. */
      const std::shared_ptr<ValueModel<Type>>& get_model() const;

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
        std::shared_ptr<ValueModel<Type>> m_model;
        std::shared_ptr<ArrayListModel> m_list_model;
        std::unordered_map<Type, int> m_indexes;
        ListView* m_list_view;
        DropDownBox* m_drop_down_box;
        boost::signals2::scoped_connection m_current_connection;

        void populate_data();
        void on_current(const Type& current);
  };

  template<typename T>
  DropDownBoxAdaptor<T>::DropDownBoxAdaptor(QWidget* parent)
    : DropDownBoxAdaptor(Type::NONE, parent) {}

  template<typename T>
  DropDownBoxAdaptor<T>::DropDownBoxAdaptor(Type current, QWidget* parent)
    : DropDownBoxAdaptor(
        std::make_shared<LocalValueModel<Type>>(std::move(current)), parent) {}

  template<typename T>
  DropDownBoxAdaptor<T>::DropDownBoxAdaptor(
      std::shared_ptr<ValueModel<Type>> model, QWidget* parent)
      : QWidget(parent),
        m_model(std::move(model)),
        m_list_model(std::make_shared<ArrayListModel>()) {
    populate_data();
    for(auto i = 0; i < m_list_model->get_size(); ++i) {
      m_indexes[m_list_model->get<Type>(i)] = i;
    }
    m_list_view = new ListView(m_list_model,
      [] (const auto& model, auto index) {
        return make_label(displayTextAny(model->at(index)));
      });
    if(auto index = m_indexes.find(m_model->get_current());
        index != m_indexes.end()) {
      m_list_view->get_current_model()->set_current(index->second);
    }
    m_drop_down_box = new DropDownBox(*m_list_view);
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins({});
    layout->addWidget(m_drop_down_box);
    m_current_connection = m_model->connect_current_signal(
      [=] (const auto& current) { on_current(current); });
    m_drop_down_box->connect_submit_signal([=] (const auto& submission) {
      m_submit_signal(std::any_cast<Type>(submission));
    });
  }

  template<typename T>
  const std::shared_ptr<ValueModel<typename DropDownBoxAdaptor<T>::Type>>&
      DropDownBoxAdaptor<T>::get_model() const {
    return m_model;
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
  void DropDownBoxAdaptor<T>::on_current(const Type& current) {
    if(auto index = m_indexes.find(current); index != m_indexes.end()) {
      m_list_view->get_current_model()->set_current(index->second);
    }
  }
}

#endif
