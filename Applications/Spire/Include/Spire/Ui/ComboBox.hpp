#ifndef SPIRE_COMBO_BOX_HPP
#define SPIRE_COMBO_BOX_HPP
#include <any>
#include <vector>
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

      /** Used to retreive potential matches to a query. */
      class QueryModel {
        public:
          virtual ~QueryModel() = default;

          /**
           * Submits a query to be asynchronously resolved.
           * @param query The query to submit.
           * @return An asynchronous list of matches to the given <i>query</i>.
           */
          virtual QtPromise<std::vector<std::any>> submit(
            const QString& query) = 0;

        private:
          QueryModel(const QueryModel&) = delete;
          QueryModel(QueryModel&&) = delete;
      };

      /**
       * Signals that the value was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const std::any& submission)>;

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
       * @param current_model The current value's model.
       * @param selection_model The selection value's model.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      ComboBox(std::shared_ptr<QueryModel> query_model,
        std::shared_ptr<CurrentModel> current_model,
        std::shared_ptr<SelectionModel> selection_model,
        ViewBuilder view_builder, QWidget* parent = nullptr);

      /** Returns the model used to query matches. */
      const std::shared_ptr<QueryModel>& get_query_model() const;

      /** Returns the current model. */
      const std::shared_ptr<CurrentModel>& get_current_model() const;

      /** Returns the selection model. */
      const std::shared_ptr<SelectionModel>& get_selection_model() const;

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
  };
}

#endif
