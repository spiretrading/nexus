#ifndef SPIRE_SECURITY_INPUT_TESTER
#define SPIRE_SECURITY_INPUT_TESTER
#include <QListWidget>
#include "Nexus/Definitions/Definitions.hpp"
#include "spire/security_input/local_security_input_model.hpp"
#include "spire/security_input_ui_tester/security_input_ui_tester.hpp"

namespace spire {

  //! Tester class for SecurityInputDialog.
  class SecurityInputTester : public QListWidget {
    public:
      
      //! Constructs an empty SecurityInputTester.
      SecurityInputTester(QWidget* parent = nullptr);

    protected:
      void closeEvent(QCloseEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      LocalSecurityInputModel m_model;
      SecurityInputDialog* m_dialog;
  };
}

#endif
