#ifndef SPIRE_SECURITY_INPUT_WINDOW_HPP
#define SPIRE_SECURITY_INPUT_WINDOW_HPP
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "spire/security_input/security_input.hpp"

namespace spire {

  //! \brief Displays a security input box within its own window.
  class security_input_window : public QWidget {
    public:

      //! Signals that editing has completed.
      /*!
        \param s The security that was input.
      */
      using commit_signal = signal<void (const Nexus::Security& s)>;

      //! Constructs a security input window.
      /*!
        \param model The model to query for securities.
      */
      security_input_window(security_input_model& model);

      //! Connects a slot to the commit signal.
      boost::signals2::connection connect_commit_signal(
        const commit_signal::slot_type& slot) const;
  };
}

#endif
