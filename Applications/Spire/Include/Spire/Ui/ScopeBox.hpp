#ifndef SPIRE_SCOPE_BOX_HPP
#define SPIRE_SCOPE_BOX_HPP
#include <QWidget>
#include "Nexus/Definitions/Scope.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/QueryModel.hpp"

namespace Spire {
  template<typename> class TagComboBox;

  /** Defines a model over a Scope. */
  using ScopeModel = ValueModel<Nexus::Scope>;

  /** Defines a local model for a Scope. */
  using LocalScopeModel = LocalValueModel<Nexus::Scope>;

  /** The type used to query for Scopes. */
  using ScopeQueryModel = QueryModel<Nexus::Scope>;

  /** Displays a TagComboBox over an open set of scope values. */
  class ScopeBox : public QWidget {
    public:

      /**
       * Signals the submission of the scope.
       * @param submission The list of values to be submitted.
       */
      using SubmitSignal = Signal<void (const Nexus::Scope& submission)>;

      /**
       * Constructs a ScopeBox using a default current model.
       * @param scopes The model used to scope queries.
       * @param parent The parent widget.
       */
      explicit ScopeBox(
        std::shared_ptr<ScopeQueryModel> scopes, QWidget* parent = nullptr);

      /**
       * Constructs a ScopeBox.
       * @param scopes The model used to scope queries.
       * @param current The current value's model.
       * @param parent The parent widget.
       */
      ScopeBox(std::shared_ptr<ScopeQueryModel> scopes,
        std::shared_ptr<ScopeModel> current, QWidget* parent = nullptr);

      /** Returns the model used to query scopes. */
      const std::shared_ptr<ScopeQueryModel>& get_scopes() const;

      /** Returns the current model. */
      const std::shared_ptr<ScopeModel>& get_current() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& placeholder);

      /** Returns <code>true</code> iff this ScopeBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the ScopeBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    private:
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<ScopeQueryModel> m_scopes;
      std::shared_ptr<ScopeModel> m_current;
      Nexus::Scope m_last_scope;
      TagComboBox<Nexus::Scope>* m_tag_combo_box;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_tag_operation_connection;

      void on_current(const Nexus::Scope& scope);
      void on_submit(
        const std::shared_ptr<ListModel<Nexus::Scope>>& submission);
      void on_tags_operation(const AnyListModel::Operation& operation);
  };
}

#endif
