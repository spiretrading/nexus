#ifndef SPIRE_COMBO_BOX_HPP
#define SPIRE_COMBO_BOX_HPP
#include <any>
#include <cstdint>
#include <boost/functional/factory.hpp>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/QueryModel.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/KeyObserver.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a data field over an open set of selectable values. */
  class AnyComboBox : public QWidget {
    public:

      /**
       * Signals that the value was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const AnyRef& submission)>;

      /**
       * Constructs a ComboBox.
       * @param query_model The model used to query matches.
       * @param current The current value's model.
       * @param input_box The input box to use.
       * @param item_builder The ListViewItemBuilder to use.
       * @param matches_builder Used to build the ListModel that keeps the list
       *        matches.
       * @param parent The parent widget.
       */
      AnyComboBox(std::shared_ptr<AnyQueryModel> query_model,
        std::shared_ptr<AnyValueModel> current, AnyInputBox* input_box,
        ListViewItemBuilder<> item_builder,
        std::function<std::shared_ptr<AnyListModel> ()> matches_builder,
        QWidget* parent = nullptr);

      /** Returns the model used to query matches. */
      const std::shared_ptr<AnyQueryModel>& get_query_model() const;

      /** Returns the current model. */
      const std::shared_ptr<AnyValueModel>& get_current() const;

      /** Returns the last submission. */
      const std::any& get_submission() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& placeholder);

      /** Returns <code>true</code> iff this DropDownBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the DropDownBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void showEvent(QShowEvent *event) override;

    private:
      template<typename> friend class ComboBox;
      struct DeferredData {
        mutable SubmitSignal m_submit_signal;
        std::any m_submission;
        QString m_submission_text;
        QWidget* m_input_focus_proxy;
        FocusObserver m_focus_observer;
        KeyObserver m_key_observer;
        std::shared_ptr<AnyListModel> m_matches;
        DropDownList* m_drop_down_list;
        EmptyState* m_empty_state;
        boost::optional<QString> m_user_query;
        std::uint32_t m_completion_tag;
        QtPromise<void> m_query_result;
        QString m_prefix;
        QString m_completion;
        QString m_last_completion;
        bool m_has_autocomplete_selection;
        bool m_is_querying;
        boost::signals2::scoped_connection m_current_connection;
        boost::signals2::scoped_connection m_input_connection;
        boost::signals2::scoped_connection m_highlight_connection;
        boost::signals2::scoped_connection m_drop_down_current_connection;

        DeferredData(AnyComboBox& box);
      };
      std::shared_ptr<AnyQueryModel> m_query_model;
      std::shared_ptr<AnyValueModel> m_current;
      AnyInputBox* m_input_box;
      ListViewItemBuilder<> m_item_builder;
      std::function<std::shared_ptr<AnyListModel> ()> m_matches_builder;
      std::unique_ptr<DeferredData> m_data;

      static std::shared_ptr<TextModel>
        make_text_wrapper_model(std::shared_ptr<AnyValueModel> current);
      void update_focus_proxy();
      void initialize_deferred_data() const;
      void update_completion();
      void revert_to(const QString& query, bool autocomplete);
      void revert_current();
      void submit(const QString& query, bool is_passive = false);
      void on_current(const std::any& current);
      void on_input(const AnyRef& current);
      void on_highlight(const Highlight& highlight);
      void on_submit(const AnyRef& query);
      void on_query(std::uint32_t tag, bool show,
        Beam::Expect<std::vector<std::any>>&& result);
      void on_drop_down_current(boost::optional<int> index);
      void on_drop_down_submit(const std::any& submission);
      void on_focus(FocusObserver::State state);
      bool on_input_key_press(QWidget& target, QKeyEvent& event);
  };

  /**
   * Displays a data field over an open set of selectable values.
   * @param <T> The type of selectable value.
   */
  template<typename T>
  class ComboBox : public AnyComboBox {
    public:

      /** The type of selectable value. */
      using Type = T;

      /** The type of QueryModel used to resolve options. */
      using QueryModel = Spire::QueryModel<Type>;

      /** The type of ValueModel used to track the current value. */
      using CurrentModel = ValueModel<Type>;

      /**
       * Signals that the value was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const Type& submission)>;

      /**
       * Constructs a ComboBox using default local models, a TextBox and
       * a default view builder.
       * @param query_model The model used to query matches.
       * @param parent The parent widget.
       */
      explicit ComboBox(
        std::shared_ptr<QueryModel> query_model, QWidget* parent = nullptr);

      /**
       * Constructs a ComboBox using default local models and a TextBox.
       * @param query_model The model used to query matches.
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      ComboBox(std::shared_ptr<QueryModel> query_model,
        ListViewItemBuilder<ListModel<Type>> item_builder,
        QWidget* parent = nullptr);

      /**
       * Constructs a ComboBox using a TextBox.
       * @param query_model The model used to query matches.
       * @param current The current value's model.
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      ComboBox(std::shared_ptr<QueryModel> query_model,
        std::shared_ptr<CurrentModel> current,
        ListViewItemBuilder<ListModel<Type>> item_builder,
        QWidget* parent = nullptr);

      /**
       * Constructs a ComboBox.
       * @param query_model The model used to query matches.
       * @param current The current value's model.
       * @param input_box The input box to use.
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      ComboBox(std::shared_ptr<QueryModel> query_model,
        std::shared_ptr<CurrentModel> current, AnyInputBox* input_box,
        ListViewItemBuilder<ListModel<Type>> item_builder,
        QWidget* parent = nullptr);

      /** Returns the model used to query matches. */
      std::shared_ptr<QueryModel> get_query_model() const;

      /** Returns the current model. */
      std::shared_ptr<CurrentModel> get_current() const;

      /** Returns the last submission. */
      const Type& get_submission() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;
  };

  template<std::derived_from<AnyQueryModel> QueryModel,
    std::derived_from<AnyValueModel> ValueModel>
  ComboBox(std::shared_ptr<QueryModel>, std::shared_ptr<ValueModel>,
    ListViewItemBuilder<ListModel<typename QueryModel::Type>>) ->
      ComboBox<typename QueryModel::Type>;

  template<typename T>
  ComboBox<T>::ComboBox(
    std::shared_ptr<QueryModel> query_model, QWidget* parent)
    : ComboBox(std::move(query_model), &ListView::default_item_builder,
        parent) {}

  template<typename T>
  ComboBox<T>::ComboBox(std::shared_ptr<QueryModel> query_model,
    ListViewItemBuilder<ListModel<Type>> item_builder, QWidget* parent)
    : ComboBox(std::move(query_model),
        std::make_shared<LocalValueModel<Type>>(), std::move(item_builder),
        parent) {}

  template<typename T>
  ComboBox<T>::ComboBox(std::shared_ptr<QueryModel> query_model,
    std::shared_ptr<CurrentModel> current,
    ListViewItemBuilder<ListModel<Type>> item_builder, QWidget* parent)
    : ComboBox(std::move(query_model), current, new AnyInputBox(
        *(new TextBox(AnyComboBox::make_text_wrapper_model(current)))),
        std::move(item_builder), parent) {}

  template<typename T>
  ComboBox<T>::ComboBox(std::shared_ptr<QueryModel> query_model,
    std::shared_ptr<CurrentModel> current, AnyInputBox* input_box,
    ListViewItemBuilder<ListModel<Type>> item_builder, QWidget* parent)
    : AnyComboBox(std::move(query_model), std::move(current), input_box,
        std::move(item_builder),
        boost::factory<std::shared_ptr<ArrayListModel<Type>>>(), parent) {}

  template<typename T>
  std::shared_ptr<typename ComboBox<T>::QueryModel>
      ComboBox<T>::get_query_model() const {
    return std::static_pointer_cast<QueryModel>(AnyComboBox::get_query_model());
  }

  template<typename T>
  std::shared_ptr<typename ComboBox<T>::CurrentModel>
      ComboBox<T>::get_current() const {
    return std::static_pointer_cast<CurrentModel>(AnyComboBox::get_current());
  }

  template<typename T>
  const typename ComboBox<T>::Type& ComboBox<T>::get_submission() const {
    return std::any_cast<const Type&>(AnyComboBox::get_submission());
  }

  template<typename T>
  boost::signals2::connection ComboBox<T>::connect_submit_signal(
      const SubmitSignal::slot_type& slot) const {
    return AnyComboBox::connect_submit_signal([=] (const auto& current) {
      slot(any_cast<Type>(current));
    });
  }
}

#endif
