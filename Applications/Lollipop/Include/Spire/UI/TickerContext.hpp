#ifndef SPIRE_TICKER_CONTEXT_HPP
#define SPIRE_TICKER_CONTEXT_HPP
#include <set>
#include <boost/signals2/signal.hpp>
#include "Nexus/Definitions/Ticker.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {
namespace UI {

  /*! \class TickerContext
      \brief Interface for a window that displays a Ticker.
   */
  class TickerContext {
    public:

      //! Signals the displayed Ticker was changed.
      /*!
        \param ticker The Ticker being displayed.
      */
      typedef boost::signals2::signal<void (const Nexus::Ticker& ticker)>
        TickerDisplaySignal;

      //! Finds the TickerContext with the specified identifier.
      /*!
        \param identifier The identifier to find.
        \return The TickerContext with the specified <i>identifier</i>.
      */
      static boost::optional<TickerContext&> FindTickerContext(
        const std::string& identifier);

      virtual ~TickerContext();

      //! Links this TickerContext to another.
      /*!
        \param context The TickerContext to link to.
      */
      void Link(TickerContext& context);

      //! Unlinks this TickerContext from another.
      void Unlink();

      //! The Ticker currently displayed.
      const Nexus::Ticker& GetDisplayedTicker() const;

      //! Returns an identifier unique to this TickerContext.
      const std::string& GetIdentifier() const;

      //! Returns the identifier that this TickerContext is linked to.
      const std::string& GetLinkedIdentifier() const;

      //! Connects a slot to the TickerDisplaySignal.
      boost::signals2::connection ConnectTickerDisplaySignal(
        const TickerDisplaySignal::slot_type& slot) const;

    protected:

      //! Constructs a TickerContext.
      TickerContext();

      //! Constructs a TickerContext with an existing identifier.
      /*!
        \param identifier The identifier.
      */
      TickerContext(const std::string& identifier);

      //! Handles a link action from this TickerContext to another.
      /*!
        \param context The TickerContext to link to.
      */
      virtual void HandleLink(TickerContext& context);

      //! Handles an unlink action of this TickerContext from another.
      virtual void HandleUnlink();

      //! Sets the Ticker being displayed.
      /*!
        \param ticker The Ticker being displayed.
      */
      void SetDisplayedTicker(const Nexus::Ticker& ticker);

    private:
      Nexus::Ticker m_ticker;
      std::string m_identifier;
      std::set<TickerContext*> m_incomingLinks;
      TickerContext* m_outgoingLink;
      mutable TickerDisplaySignal m_tickerDisplaySignal;
  };
}
}

#endif
