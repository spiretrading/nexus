#ifndef SPIRE_SECURITY_INPUT_TESTER
#define SPIRE_SECURITY_INPUT_TESTER
#include <QListWidget>
#include "Nexus/Definitions/Definitions.hpp"
#include "Spire/SecurityInput/LocalSecurityInputModel.hpp"
#include "Spire/SecurityInputUiTester/SecurityInputUiTester.hpp"

namespace Spire {

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
