#ifndef SPIRE_LINEINPUTDIALOG_HPP
#define SPIRE_LINEINPUTDIALOG_HPP
#include <memory>
#include <QDialog>
#include "Spire/UI/UI.hpp"

class Ui_LineInputDialog;

namespace Spire {
namespace UI {

  /*! \class LineInputDialog
      \brief Prompts the user for a single line of text.
   */
  class LineInputDialog : public QDialog {
    public:

      //! Constructs a LineInputDialog.
      /*!
        \param title The window's title.
        \param inputLabel The label to display next to the input.
        \param initialInput The initial input to display.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      LineInputDialog(const std::string& title, const std::string& inputLabel,
        const std::string& initialInput, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~LineInputDialog();

      //! Returns the input provided by the user.
      std::string GetInput() const;

    private:
      std::unique_ptr<Ui_LineInputDialog> m_ui;
  };
}
}

#endif
