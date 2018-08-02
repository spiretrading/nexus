#ifndef SPIRE_OVERLAY_WIDGET_HPP
#define SPIRE_OVERLAY_WIDGET_HPP
#include <QWidget>

namespace spire {

  //! Displays a widget on top of another widget.
  class OverlayWidget : public QWidget {
    public:

      //! Constructs an overlay widget.
      /*
        \param covered The widget that the displayed widget is drawn on top of.
        \param displayed The widget to display on top of the covered widget.
        \param parent The parent widget.
      */
      OverlayWidget(QWidget* covered, QWidget* displayed,
        QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      QWidget* m_covered;
      QWidget* m_displayed;

      void align();
  };
}

#endif
