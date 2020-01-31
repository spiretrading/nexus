#ifndef SPIRE_INPUT_WIDGET_HPP
#define SPIRE_INPUT_WIDGET_HPP
#include <QLineEdit>

namespace Spire {

  //! Displays a line edit with the Spire style.
  class InputWidget : public QLineEdit {
    public:

      //! Constructs an InputWidget.
      /*
        \param parent The parent widget.
      */
      explicit InputWidget(QWidget* parent = nullptr);
  };
}

#endif
