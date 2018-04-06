#ifndef SPIRE_DROP_SHADOW
#define SPIRE_DROP_SHADOW
#include <QWidget>
#include "spire/ui/ui.hpp"

namespace spire {

  //! Paints a drop shadow around its parent widget.
  class drop_shadow : public QWidget {
    public:

      //! Constructs a drop shadow.
      /*!
        \param parent The parent to add the drop shadow to.
      */
      drop_shadow(QWidget* parent);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void hideEvent(QHideEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      bool m_is_visible;

      void follow_parent();
  };
}

#endif
