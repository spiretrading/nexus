#ifndef SPIRE_TICKER_DIALOG_HPP
#define SPIRE_TICKER_DIALOG_HPP
#include <QWidget>
#include "Spire/Ui/TickerBox.hpp"

namespace Spire {
  class OverlayPanel;

  /**
   * Displays a dialog containing a TickerBox to allow a user to enter a
   * ticker.
   */
  class TickerDialog : public QWidget {
    public:
      using SubmitSignal = TickerBox::SubmitSignal;

      /**
       * Constructs a TickerDialog.
       * @param tickers The set of tickers that can be queried.
       * @param parent The parent widget.
       */
      explicit TickerDialog(std::shared_ptr<TickerInfoQueryModel> tickers,
        QWidget* parent = nullptr);

      /**
       * Constructs a TickerDialog.
       * @param tickers The set of tickers that can be queried.
       * @param current The current ticker to display.
       * @param parent The parent widget.
       */
      explicit TickerDialog(std::shared_ptr<TickerInfoQueryModel> tickers,
        std::shared_ptr<TickerModel> current, QWidget* parent = nullptr);

      /** Returns the set of tickers that can be queried. */
      const std::shared_ptr<TickerInfoQueryModel>& get_tickers() const;

      /** Returns the current ticker. */
      const std::shared_ptr<TickerModel>& get_current() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      TickerBox* m_ticker_box;
      OverlayPanel* m_panel;
      QWidget* m_input_box;
  };
}

#endif
