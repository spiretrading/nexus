#ifndef SPIRE_TAG_COMBO_BOX_HPP
#define SPIRE_TAG_COMBO_BOX_HPP
#include <boost/functional/factory.hpp>
#include <QWidget>
#include "Spire/Spire/AnyRef.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/QueryModel.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ListViewItemBuilder.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a ComboBox using the TagBox as the input box. */
  class AnyTagComboBox : public QWidget {
    public:

      /**
       * Signals the submission of a list of tags.
       * @param submission The list of values submitted.
       */
      using SubmitSignal =
        Signal<void (const std::shared_ptr<AnyListModel>& submission)>;

      /**
       * Constructs an AnyTagComboBox.
       * @param query_model The model used to query matches.
       * @param current The current model which holds a list of tags.
       * @param submission_builder The model used to build the list of submitted
       *        tags.
       * @param item_builder The ListViewItemBuilder to use.
       * @param matches_builder Used to build the ListModel that keeps the list
       *        matches.
       * @param parent The parent widget.
       */
      AnyTagComboBox(std::shared_ptr<AnyQueryModel> query_model,
        std::shared_ptr<AnyListModel> current,
        std::function<std::shared_ptr<AnyListModel> ()> submission_builder,
        ListViewItemBuilder<> item_builder,
        std::function<std::shared_ptr<AnyListModel> ()> matches_builder,
        QWidget* parent = nullptr);

      /** Returns the model used to query matches. */
      const std::shared_ptr<AnyQueryModel>& get_query_model() const;

      /** Returns the current list of tags. */
      const std::shared_ptr<AnyListModel>& get_current() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& placeholder);

      /** Returns <code>true</code> iff this TagComboBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the TagComboBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      TagBox* m_tag_box;
      AnyComboBox* m_combo_box;
      std::shared_ptr<AnyListModel> m_submission;
      FocusObserver m_focus_observer;
      QWidget* m_input_box;
      AnyInputBox* m_any_input_box;
      QWidget* m_drop_down_window;
      bool m_is_modified;
      boost::signals2::scoped_connection m_list_connection;

      void install_text_proxy_event_filter();
      void push_combo_box();
      void submit();
      QWidget* find_drop_down_window();
      void on_combo_box_submit(const AnyRef& submission);
      void on_focus(FocusObserver::State state);
      void on_operation(const AnyListModel::Operation& operation);
  };

  /**
   * Displays a ComboBox using the TagBox as the input box.
   * @param <T> The type of value each tag represents.
   */
  template<typename T>
  class TagComboBox : public AnyTagComboBox {
    public:

      /** The type of value each tag represents. */
      using Type = T;

      /** The type of QueryModel used to resolve options. */
      using QueryModel = Spire::QueryModel<Type>;

      /** The type of ValueModel used to track the current value. */
      using CurrentModel = ValueModel<Type>;

      /**
       * Signals the submission of a list of tags.
       * @param submission The list of values to be submitted.
       */
      using SubmitSignal =
        Signal<void (const std::shared_ptr<ListModel<Type>>& submission)>;

      /**
       * Constructs a TagComboBox using a default local model and a default view
       * builder.
       * @param query_model The model used to query matches.
       * @param parent The parent widget.
       */
      explicit TagComboBox(std::shared_ptr<QueryModel> query_model,
        QWidget* parent = nullptr);

      /**
       * Constructs a TagComboBox using a default local model.
       * @param query_model The model used to query matches.
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      TagComboBox(std::shared_ptr<QueryModel> query_model,
        ListViewItemBuilder<ListModel<Type>> item_builder,
        QWidget* parent = nullptr);

      /**
       * Constructs a TagComboBox.
       * @param query_model The model used to query matches.
       * @param current The current model which holds a list of tags.
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      TagComboBox(std::shared_ptr<QueryModel> query_model,
        std::shared_ptr<ListModel<Type>> current,
        ListViewItemBuilder<ListModel<Type>> item_builder,
        QWidget* parent = nullptr);

      /** Returns the model used to query matches. */
      std::shared_ptr<QueryModel> get_query_model() const;

      /** Returns the current list of tags. */
      std::shared_ptr<ListModel<Type>> get_current() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;
  };

  template<std::derived_from<AnyQueryModel> QueryModel>
  TagComboBox(std::shared_ptr<QueryModel>) ->
    TagComboBox<typename QueryModel::Type>;

  template<std::derived_from<AnyQueryModel> QueryModel,
    std::derived_from<AnyListModel> ListModel, typename U>
  TagComboBox(std::shared_ptr<QueryModel>, std::shared_ptr<ListModel>,
    U) -> TagComboBox<typename QueryModel::Type>;


  template<typename T>
  TagComboBox<T>::TagComboBox(
    std::shared_ptr<QueryModel> query_model, QWidget* parent)
    : TagComboBox(
        std::move(query_model), &ListView::default_item_builder, parent) {}

  template<typename T>
  TagComboBox<T>::TagComboBox(std::shared_ptr<QueryModel> query_model,
    ListViewItemBuilder<ListModel<Type>> item_builder, QWidget* parent)
    : TagComboBox(
        std::move(query_model), std::make_shared<ArrayListModel<Type>>(),
        std::move(item_builder), parent) {}

  template<typename T>
  TagComboBox<T>::TagComboBox(std::shared_ptr<QueryModel> query_model,
    std::shared_ptr<ListModel<Type>> current,
    ListViewItemBuilder<ListModel<Type>> item_builder, QWidget* parent)
    : AnyTagComboBox(std::move(query_model), std::move(current),
        boost::factory<std::shared_ptr<ArrayListModel<Type>>>(),
        std::move(item_builder),
        boost::factory<std::shared_ptr<ArrayListModel<Type>>>(), parent) {}

  template<typename T>
  std::shared_ptr<typename TagComboBox<T>::QueryModel>
      TagComboBox<T>::get_query_model() const {
    return std::static_pointer_cast<QueryModel>(
      AnyTagComboBox::get_query_model());
  }

  template<typename T>
  std::shared_ptr<ListModel<typename TagComboBox<T>::Type>>
      TagComboBox<T>::get_current() const {
    return std::static_pointer_cast<ListModel<Type>>(
      AnyTagComboBox::get_current());
  }

  template<typename T>
  boost::signals2::connection TagComboBox<T>::connect_submit_signal(
      const SubmitSignal::slot_type& slot) const {
    return AnyTagComboBox::connect_submit_signal([=] (const auto& submission) {
      slot(std::static_pointer_cast<ListModel<Type>>(submission));
    });
  }
}

#endif
