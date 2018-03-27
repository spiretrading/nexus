#ifndef SPIRE_SECURITY_INPUT_DIALOG_HPP
#define SPIRE_SECURITY_INPUT_DIALOG_HPP
#include <QDialog>
#include "Nexus/Definitions/Security.hpp"
#include "spire/security_input/security_input.hpp"

namespace spire {

  //! \brief Displays a security input box within a dialog window.
  class security_input_dialog : public QDialog {
    public:

      //! Constructs a security input dialog.
      /*!
        \param model The model to query for securities.
      */
      security_input_dialog(security_input_model& model);

      //! Returns the security that was input.
      const Nexus::Security& get_security() const noexcept;

    private:
      Nexus::Security m_security;
  };
}

#endif
