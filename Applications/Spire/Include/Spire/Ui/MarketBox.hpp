#ifndef SPIRE_MARKET_BOX_HPP
#define SPIRE_MARKET_BOX_HPP
#include "Nexus/Definitions/Market.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a market over an open set of market values. */
  class MarketBox : public QWidget {
    public:

      /** A ValueModel over a Nexus::MarketCode. */
      using CurrentModel = ValueModel<Nexus::MarketCode>;

      /**
       * Signals that the value was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const Nexus::MarketCode& submission)>;

      /**
       * Constructs a MarketBox using a default local model.
       * @param query_model The model used to query matches.
       * @param parent The parent widget.
       */
      explicit MarketBox(std::shared_ptr<ComboBox::QueryModel> query_model,
        QWidget* parent = nullptr);

      /**
       * Constructs a MarketBox.
       * @param query_model The model used to query matches.
       * @param current The current value's model.
       * @param parent The parent widget.
       */
      MarketBox(std::shared_ptr<ComboBox::QueryModel> query_model,
        std::shared_ptr<CurrentModel> current, QWidget* parent = nullptr);

      /** Returns the model used to query matches. */
      const std::shared_ptr<ComboBox::QueryModel>& get_query_model() const;

      /** Returns the current model. */
      const std::shared_ptr<CurrentModel>& get_current() const;

      /** Returns the last submission. */
      const Nexus::MarketCode& get_submission() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& placeholder);

      /** Returns <code>true</code> iff this MarketBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the MarketBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    private:
      struct MarketQueryModel;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<MarketQueryModel> m_query_model;
      std::shared_ptr<CurrentModel> m_current;
      DestinationBox* m_destination_box;
      Nexus::MarketCode m_submission;
  };
}

#endif
