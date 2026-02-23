#ifndef SPIRE_TICKER_INPUT_WIDGET_HPP
#define SPIRE_TICKER_INPUT_WIDGET_HPP
#include <QWidget>
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/signal.hpp>
#include "Nexus/Definitions/Ticker.hpp"
#include "Spire/UI/UI.hpp"

class QLineEdit;

namespace Spire {

  /** Displays a widget to input and display a Ticker. */
  class TickerInputWidget : public QWidget {
    public:

      /**
       * Signals a change to the Ticker.
       * @param value The updated Ticker.
       */
      using TickerUpdatedSignal =
        boost::signals2::signal<void (const Nexus::Ticker& value)>;

      /**
       * Constructs a TickerInputWidget.
       * @param parent The parent widget.
       * @param flags Qt flags passed to the parent widget.
       */
      explicit TickerInputWidget(
        QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

      /**
       * Constructs a TickerInputWidget.
       * @param userProfile The user's profile.
       * @param parent The parent widget.
       * @param flags Qt flags passed to the parent widget.
       */
      explicit TickerInputWidget(Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

      /**
       * Initializes this widget.
       * @param userProfile The user's profile.
       */
      void Initialize(Beam::Ref<UserProfile> userProfile);

      /** Returns the Ticker displayed. */
      const Nexus::Ticker& GetTicker() const;

      /**
       * Sets the Ticker to display.
       * @param ticker The Ticker to display.
       */
      void SetTicker(Nexus::Ticker ticker);

      /**
       * Makes this widget read-only.
       * @param value <code>true</code> to make this widget read-only.
       */
      void SetReadOnly(bool value);

      /**
       * Connects a slot to the TickerUpdatedSignal.
       * @param slot The slot to connect.
       * @return A connection to the signal.
       */
      boost::signals2::connection ConnectTickerUpdatedSignal(
        const TickerUpdatedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* receiver, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mouseDoubleClickEvent(QMouseEvent* event) override;

    private:
      UserProfile* m_userProfile;
      Nexus::Ticker m_ticker;
      QLineEdit* m_lineEdit;
      bool m_isReadOnly;
      mutable TickerUpdatedSignal m_tickerUpdatedSignal;
  };
}

#endif
