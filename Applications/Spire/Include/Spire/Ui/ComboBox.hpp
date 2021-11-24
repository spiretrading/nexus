#ifndef SPIRE_COMBO_BOX_HPP
#define SPIRE_COMBO_BOX_HPP
#include <any>
#include <vector>
#include <Beam/Collections/Trie.hpp>
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a data field over an open set of selectable values. */
  class ComboBox : public QWidget {
    public:

      /** The type of model representing the index of the current value. */
      using CurrentModel = ListView::CurrentModel;

      /** The type of model representing the index of the selected value. */
      using SelectionModel = ListView::SelectionModel;

      /** The type of function used to build a QWidget representing a value. */
      using ViewBuilder = ListView::ViewBuilder;

      /**
       * Signals that the value was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const std::any& submission)>;

      /** Used to retreive potential matches to a query. */
      class QueryModel {
        public:
          virtual ~QueryModel() = default;

          /**
           * Parses a value from a query string.
           * @param query The query string to parse.
           * @return The value represented by the <i>query</i> or an empty
           *         object if the <i>query</i> does not represent a valid
           *         value.
           */
          virtual std::any parse(const QString& query) = 0;

          /**
           * Submits a query to be asynchronously resolved.
           * @param query The query to submit.
           * @return An asynchronous list of matches to the given <i>query</i>.
           */
          virtual QtPromise<std::vector<std::any>> submit(
            const QString& query) = 0;

        protected:

          /** Constructs a QueryModel. */
          QueryModel() = default;

        private:
          QueryModel(const QueryModel&) = delete;
          QueryModel& operator =(const QueryModel&) = delete;
      };

      /**
       * Constructs a ComboBox using default local models and a default view
       * builder.
       * @param query_model The model used to query matches.
       * @param parent The parent widget.
       */
      explicit ComboBox(
        std::shared_ptr<QueryModel> query_model, QWidget* parent = nullptr);

      /**
       * Constructs a ComboBox using default local models.
       * @param query_model The model used to query matches.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      ComboBox(std::shared_ptr<QueryModel> query_model,
        ViewBuilder view_builder, QWidget* parent = nullptr);

      /**
       * Constructs a ComboBox.
       * @param query_model The model used to query matches.
       * @param current The current value's model.
       * @param selection The selection value's model.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      ComboBox(std::shared_ptr<QueryModel> query_model,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<SelectionModel> selection, ViewBuilder view_builder,
        QWidget* parent = nullptr);

      /** Returns the model used to query matches. */
      const std::shared_ptr<QueryModel>& get_query_model() const;

      /** Returns the current model. */
      const std::shared_ptr<CurrentModel>& get_current() const;

      /** Returns the selection model. */
      const std::shared_ptr<SelectionModel>& get_selection() const;

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

    private:
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<QueryModel> m_query_model;
      bool m_is_read_only;
      TextBox* m_input_box;
      ListView* m_list_view;
      std::shared_ptr<ArrayListModel> m_matches;
      DropDownList* m_drop_down_list;
      QtPromise<std::vector<std::any>> m_query_result;
      boost::signals2::scoped_connection m_input_connection;

      void on_input(const QString& query);
      void on_query(Beam::Expect<std::vector<std::any>>&& result);
  };

  /**
   * Implements an in-memory QueryModel associating values with a string
   * representation.
   */
  class LocalComboBoxQueryModel : public ComboBox::QueryModel {
    public:

      /** Constructs an empty model. */
      LocalComboBoxQueryModel();

      /**
       * Adds a value to the model that can be queried through its string
       * representation.
       * @param value The value to add.
       */
      void add(const std::any& value);

      /**
       * Adds a value to the model that can be queried by a given string.
       * @param id The string used to query the value.
       * @param value The value to add.
       */
      void add(const QString& id, const std::any& value);

      std::any parse(const QString& query) override;

      QtPromise<std::vector<std::any>> submit(const QString& query) override;

    private:
      rtv::Trie<QChar, std::any> m_values;
  };
}

#endif
