#ifndef SPIRE_LOGIN_CONTROLLER_HPP
#define SPIRE_LOGIN_CONTROLLER_HPP
#include <functional>
#include <memory>
#include <string>
#include <boost/noncopyable.hpp>
#include "Nexus/ServiceClients/ServiceClients.hpp"
#include "spire/login/login.hpp"
#include "spire/spire/qt_promise.hpp"

namespace spire {

  //! \brief Allows the user to login to Spire.
  class login_controller : private boost::noncopyable {
    public:

      //! Signals a successful login.
      using logged_in_signal = signal<void ()>;

      //! Factory used to build the Spire service clients.
      /*!
        \param username The username to login with.
        \param password The password to login with.
        \return The service clients used to login to Spire.
      */
      using service_clients_factory =
        std::function<std::unique_ptr<Nexus::VirtualServiceClients>
        (const std::string& username, const std::string& password)>;

      //! Constructs a login controller in a state ready to display the login
      //! window.
      /*!
        \param service_clients_factory Builds the service clients used to
               login to Spire.
      */
      login_controller(service_clients_factory service_clients_factory);

      ~login_controller();

      //! Returns the service clients that logged in.
      std::unique_ptr<Nexus::VirtualServiceClients>& get_service_clients();

      //! Launches the login window.
      void open();

      //! Connects a slot to the logged in signal.
      boost::signals2::connection connect_logged_in_signal(
        const logged_in_signal::slot_type& slot) const;

    private:
      mutable logged_in_signal m_logged_in_signal;
      service_clients_factory m_service_clients_factory;
      std::unique_ptr<login_window> m_login_window;
      qt_promise m_login_promise;
      std::unique_ptr<Nexus::VirtualServiceClients> m_service_clients;

      void on_login(const std::string& username, const std::string& password);
      void on_cancel();
      void on_login_promise(Beam::Expect<
        std::unique_ptr<Nexus::VirtualServiceClients>> service_clients);
  };
}

#endif
