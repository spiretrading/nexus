#ifndef SPIRE_SALE_CONDITION_BOX_HPP
#define SPIRE_SALE_CONDITION_BOX_HPP
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a sale condition over a set of conditions. */
  class SaleConditionBox : public QWidget {
    public:

      /** A ValueModel over a sale condition. */
      using CurrentModel = ValueModel<Nexus::TimeAndSale::Condition>;

      /**
       * Signals that the value was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (
        const Nexus::TimeAndSale::Condition& submission)>;

      /**
       * Constructs a SaleConditionBox using a default local model.
       * @param query_model The model used to query matches.
       * @param parent The parent widget.
       */
      explicit SaleConditionBox(
        std::shared_ptr<ComboBox::QueryModel> query_model,
        QWidget* parent = nullptr);

      /**
       * Constructs a SaleConditionBox.
       * @param query_model The model used to query matches.
       * @param current The current value's model.
       * @param parent The parent widget.
       */
      SaleConditionBox(std::shared_ptr<ComboBox::QueryModel> query_model,
        std::shared_ptr<CurrentModel> current, QWidget* parent = nullptr);

      /** Returns the model used to query matches. */
      const std::shared_ptr<ComboBox::QueryModel>& get_query_model() const;

      /** Returns the current model. */
      const std::shared_ptr<CurrentModel>& get_current() const;

      /** Returns the last submission. */
      const Nexus::TimeAndSale::Condition& get_submission() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& placeholder);

      /** Returns <code>true</code> iff this SaleConditionBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the SaleConditionBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      struct SaleConditionQueryModel;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<SaleConditionQueryModel> m_query_model;
      std::shared_ptr<CurrentModel> m_current;
      Nexus::TimeAndSale::Condition m_submission;
      ComboBox* m_combo_box;
      AnyInputBox* m_input_box;
      bool m_is_rejected;
      boost::signals2::scoped_connection m_current_connection;

      void on_current(const Nexus::TimeAndSale::Condition& current);
      void on_input_submit(const AnyRef& submission);
      void on_submit(const std::any& submission);
  };
}

#endif
