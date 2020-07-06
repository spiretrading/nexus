#ifndef SPIRE_DROP_DOWN_WINDOW_HPP
#define SPIRE_DROP_DOWN_WINDOW_HPP
#include <QWidget>

namespace Spire {

  //! Represents a drop down window for use with a button.
  class DropDownWindow : public QWidget {
    public:

      //! Constructs a DropDownWindow.
      /*!
        \param parent The parent widget that the DropDownWindow will be shown
                      relative to.
      */
      explicit DropDownWindow(QWidget* parent = nullptr);
  };
}

#endif
