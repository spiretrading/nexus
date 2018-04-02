#ifndef SPIRE_SECURITY_INPUT_TESTER
#define SPIRE_SECURITY_INPUT_TESTER
#include <QListWidget>
#include "Nexus/Definitions/Security.hpp"
#include "spire/security_input/local_security_input_model.hpp"
#include "spire/security_input/security_input_dialog.hpp"

namespace spire {

  //! \brief Tester class for security_input_dialog.
  class security_input_tester : public QListWidget {
    public:
      
      //! Constructs an empty security_input_tester.
      security_input_tester(QWidget* parent = nullptr);

      //! Destructs the security_input_tester.
      ~security_input_tester();

      //! Adds a security to the list.
      /*!
        \param security The security to add.
      */
      void add_security(const Nexus::Security& security);

    protected:
      void closeEvent(QCloseEvent* event) override;

    private:
      local_security_input_model m_model;
      security_input_dialog* m_dialog;

      void run_dialog();
  };
}

#endif
