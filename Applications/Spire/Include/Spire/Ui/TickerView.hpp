#ifndef SPIRE_TICKER_VIEW_HPP
#define SPIRE_TICKER_VIEW_HPP
#include <vector>
#include <QStackedWidget>
#include "Spire/Ui/TickerDialog.hpp"

namespace Spire {

  /**
   * Displays tickers and provides the functionality to quickly search and
   * cycle through tickers.
   */
  class TickerView : public QWidget {
    public:

      /** A ValueModel over a Nexus::Ticker. */
      using CurrentModel = ValueModel<Nexus::Ticker>;

      /**
       * Constructs a TickerView using a default local model.
       * @param tickers The set of tickers that can be queried.
       * @param body The component that represents the current ticker.
       * @param parent The parent widget.
       */
      TickerView(std::shared_ptr<TickerInfoQueryModel> tickers, QWidget& body,
        QWidget* parent = nullptr);

      /**
       * Constructs a TickerView.
       * @param tickers The set of tickers that can be queried.
       * @param current The current value's model.
       * @param body The component that represents the current ticker.
       * @param parent The parent widget.
       */
      TickerView(std::shared_ptr<TickerInfoQueryModel> tickers,
        std::shared_ptr<CurrentModel> current, QWidget& body,
        QWidget* parent = nullptr);

      /** Returns the set of tickers that can be queried. */
      const std::shared_ptr<TickerInfoQueryModel>& get_tickers() const;

      /** Returns the current ticker displayed. */
      const std::shared_ptr<CurrentModel>& get_current() const;

      /** Returns the body. */
      const QWidget& get_body() const;

      /** Returns the body. */
      QWidget& get_body();

    protected:
      void keyPressEvent(QKeyEvent* event) override;

    private:
      TickerDialog m_ticker_dialog;
      std::shared_ptr<CurrentModel> m_current;
      QWidget* m_body;
      QStackedWidget* m_layers;
      std::vector<Nexus::Ticker> m_tickers;
      int m_current_index;

      void on_submit(const Nexus::Ticker& ticker);
  };
}

#endif
