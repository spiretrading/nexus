#ifndef SPIRE_BOOK_VIEW_MODEL_HPP
#define SPIRE_BOOK_VIEW_MODEL_HPP
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "spire/book_view/book_view.hpp"

namespace spire {

  //! Models a security's quotes.
  class book_view_model : private boost::noncopyable {
    public:

      //! Signals an update to the BBO.
      using bbo_signal = signal<void (const Nexus::BboQuote& bbo)>;

      //! Signals a price update.
      using price_signal = signal<void (Nexus::Money value)>;

      //! Signals a quantity update.
      using quantity_signal = signal<void (Nexus::Quantity value)>;

      virtual ~book_view_model() = default;

      //! Returns the security being modeled.
      virtual const Nexus::Security& get_security() const = 0;

      //! Returns the BboQuote.
      virtual const Nexus::BboQuote& get_bbo() const = 0;

      //! Returns the highest price of the session.
      virtual Nexus::Money get_high() const = 0;

      //! Returns the lowest price of the session.
      virtual Nexus::Money get_low() const = 0;

      //! Returns the session's opening price.
      virtual Nexus::Money get_open() const = 0;

      //! Returns the previous session's closing price.
      virtual Nexus::Money get_close() const = 0;

      //! Returns the session's volume.
      virtual Nexus::Quantity get_volume() const = 0;

      //! Connects a slot to the bbo signal.
      /*!
        \param slot The slot to connect.
      */
      virtual boost::signals2::connection connect_bbo_slot(
        const bbo_signal::slot_type& slot) const = 0;

      //! Connects a slot to the high signal.
      /*!
        \param slot The slot to connect.
      */
      virtual boost::signals2::connection connect_high_slot(
        const price_signal::slot_type& slot) const = 0;

      //! Connects a slot to the low signal.
      /*!
        \param slot The slot to connect.
      */
      virtual boost::signals2::connection connect_low_slot(
        const price_signal::slot_type& slot) const = 0;

      //! Connects a slot to the open signal.
      /*!
        \param slot The slot to connect.
      */
      virtual boost::signals2::connection connect_open_slot(
        const price_signal::slot_type& slot) const = 0;

      //! Connects a slot to the close signal.
      /*!
        \param slot The slot to connect.
      */
      virtual boost::signals2::connection connect_close_slot(
        const price_signal::slot_type& slot) const = 0;

      //! Connects a slot to the volume signal.
      /*!
        \param slot The slot to connect.
      */
      virtual boost::signals2::connection connect_volume_slot(
        const quantity_signal::slot_type& slot) const = 0;

    protected:

      //! Constructs a book view model.
      book_view_model() = default;
  };
}

#endif
