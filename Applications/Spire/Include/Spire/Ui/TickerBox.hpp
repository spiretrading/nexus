#ifndef SPIRE_TICKER_BOX_HPP
#define SPIRE_TICKER_BOX_HPP
#include <QWidget>
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/Definitions/TickerInfo.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/QueryModel.hpp"

namespace Spire {
  template<typename> class ComboBox;

  /** Represents a ValueModel for a Ticker. */
  using TickerModel = ValueModel<Nexus::Ticker>;

  /** Represents a LocalValueModel for a Ticker. */
  using LocalTickerModel = LocalValueModel<Nexus::Ticker>;

  /** Represents the QueryModel used for TickerInfo objects. */
  using TickerInfoQueryModel = QueryModel<Nexus::TickerInfo>;

  /** Displays a ticker over an open set of ticker values. */
  class TickerBox : public QWidget {
    public:

      /** A ValueModel over a Nexus::Ticker. */
      using CurrentModel = TickerModel;

      /**
       * Signals that the value was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const Nexus::Ticker& submission)>;

      /**
       * Constructs a TickerBox using a default local model.
       * @param tickers The set of tickers that can be queried.
       * @param parent The parent widget.
       */
      explicit TickerBox(std::shared_ptr<TickerInfoQueryModel> tickers,
        QWidget* parent = nullptr);

      /**
       * Constructs a TickerBox.
       * @param tickers The set of tickers that can be queried.
       * @param current The current ticker displayed.
       * @param parent The parent widget.
       */
      TickerBox(std::shared_ptr<TickerInfoQueryModel> tickers,
        std::shared_ptr<CurrentModel> current, QWidget* parent = nullptr);

      /** Returns the set of tickers that can be queried. */
      const std::shared_ptr<TickerInfoQueryModel>& get_tickers() const;

      /** Returns the current model. */
      const std::shared_ptr<CurrentModel>& get_current() const;

      /** Returns the last submission. */
      const Nexus::Ticker& get_submission() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& placeholder);

      /** Returns <code>true</code> iff this TickerBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the TickerBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    private:
      struct TickerQueryModel;
      std::shared_ptr<TickerQueryModel> m_tickers;
      std::shared_ptr<CurrentModel> m_current;
      ComboBox<Nexus::Ticker>* m_combo_box;
  };
}

#endif
