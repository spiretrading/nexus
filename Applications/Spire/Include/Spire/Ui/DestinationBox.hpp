#ifndef SPIRE_DESTINATION_BOX_HPP
#define SPIRE_DESTINATION_BOX_HPP
#include "Nexus/Definitions/Destination.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Defines a model over a Destination. */
  using DestinationModel = ValueModel<Nexus::Destination>;

  /** Defines a local model for a Destination. */
  using LocalDestinationModel = LocalValueModel<Nexus::Destination>;

  /** Displays a destination over an open set of destination values. */
  class DestinationBox : public QWidget {
    public:

      /**
       * Signals that the value was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const Nexus::Destination& submission)>;

      /**
       * Constructs a DestinationBox using a default local model.
       * @param query_model The model used to query matches.
       * @param parent The parent widget.
       */
      explicit DestinationBox(std::shared_ptr<ComboBox::QueryModel> query_model,
        QWidget* parent = nullptr);

      /**
       * Constructs a DestinationBox.
       * @param query_model The model used to query matches.
       * @param current The current value's model.
       * @param parent The parent widget.
       */
      DestinationBox(std::shared_ptr<ComboBox::QueryModel> query_model,
        std::shared_ptr<DestinationModel> current, QWidget* parent = nullptr);

      /** Returns the model used to query matches. */
      const std::shared_ptr<ComboBox::QueryModel>& get_query_model() const;

      /** Returns the current model. */
      const std::shared_ptr<DestinationModel>& get_current() const;

      /** Returns the last submission. */
      const Nexus::Destination& get_submission() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& placeholder);

      /** Returns <code>true</code> iff this DestinationBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the DestinationBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      struct DestinationQueryModel;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<DestinationQueryModel> m_query_model;
      std::shared_ptr<DestinationModel> m_current;
      Nexus::Destination m_submission;
      ComboBox* m_combo_box;
      AnyInputBox* m_input_box;
      bool m_is_rejected;
      bool m_has_submit;
      boost::signals2::scoped_connection m_current_connection;

      void on_current(const Nexus::Destination& current);
      void on_input_submit(const AnyRef& submission);
      void on_submit(const std::any& submission);
  };
}

#endif
