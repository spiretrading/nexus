#ifndef SPIRE_BOOK_VIEW_CONTROLLER_HPP
#define SPIRE_BOOK_VIEW_CONTROLLER_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "Spire/BookView/BookView.hpp"
#include "Spire/SecurityInput/SecurityInput.hpp"
#include "Spire/Spire/Definitions.hpp"

namespace Spire {

  //! Manages a BookViewWindow.
  class BookViewController : private boost::noncopyable {
    public:

      //! Signals that the window closed.
      using ClosedSignal = Signal<void ()>;

      //! Constructs a BookViewController.
      /*!
        \param definitions The set of definitions to use.
        \param security_input_model The model used to autocomplete securities.
        \param service_clients The service clients logged into Spire.
      */
      BookViewController(Definitions definitions,
        Beam::Ref<SecurityInputModel> security_input_model,
        Beam::Ref<Nexus::VirtualServiceClients> service_clients);

      ~BookViewController();

      //! Displays the window.
      void open();

      //! Closes the window.
      void close();

      //! Connects a slot to the closed signal.
      boost::signals2::connection connect_closed_signal(
        const ClosedSignal::slot_type& slot) const;

    private:
      mutable ClosedSignal m_closed_signal;
      Definitions m_definitions;
      SecurityInputModel* m_security_input_model;
      Nexus::VirtualServiceClients* m_service_clients;
      BookViewWindow* m_window;
      boost::signals2::scoped_connection m_security_change_connection;

      void on_change_security(const Nexus::Security& security);
      void on_closed();
  };
}

#endif
