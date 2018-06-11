#ifndef SPIRE_LOADING_WIDGET
#define SPIRE_LOADING_WIDGET
#include <QLabel>
#include <QWidget>

namespace spire {

  //! Displays a loading widget with an animated spire logo.
  class loading_widget : public QWidget {
    public:

      //! Constructs a loading_widget.
      /*
        \param parent The parent widget.
      */
      loading_widget(QWidget* parent = nullptr);

    private:
      QLabel* m_logo_widget;
  };
}

#endif
