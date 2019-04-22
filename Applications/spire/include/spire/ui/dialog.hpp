#ifndef SPIRE_DIALOG_HPP
#define SPIRE_DIALOG_HPP
#include <QDialog>
#include "spire/ui/window.hpp"

namespace Spire {

  //! Displays a dialog that signals when it has been accepted or rejected.
  class Dialog : public Window {
    public:

      //! Constructs a Dialog.
      /*
        \param parent The parent widget to the Dialog.
      */
      Dialog(QWidget* parent = nullptr);

      //! Closes the dialog and sets the result to Accepted.
      void accept();

      //! Displays the Dialog and blocks until the Dialog closes.
      QDialog::DialogCode exec();

      //! Closes the dialog and sets the result to Rejected.
      void reject();

    private:
      QDialog::DialogCode m_result;
  };
}

#endif
