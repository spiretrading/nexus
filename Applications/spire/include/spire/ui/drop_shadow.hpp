#ifndef SPIRE_DROP_SHADOW
#define SPIRE_DROP_SHADOW
#include <QWidget>
#include "spire/ui/ui.hpp"

namespace Spire {

  //! Paints a drop shadow around its parent widget.
  class DropShadow : public QWidget {
    public:

      //! Constructs a drop shadow.
      /*!
        \param parent The parent to add the drop shadow to.
      */
      DropShadow(QWidget* parent);

      //! Constructs a drop shadow.
      /*!
        \param has_top True if the top gradient is drawn.
        \param parent The parent to add the drop shadow to.
      */
      DropShadow(bool has_top, QWidget* parent);

      //! Constructs a drop shadow.
      /*!
        \param is_menu_shadow True if the drop shadow is being used with a
                              menu.
        \param has_top True if the top gradient is drawn.
        \param parent The parent to add the drop shadow to.
      */
      DropShadow(bool is_menu_shadow, bool has_top, QWidget* parent);

    protected:
      bool event(QEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      void hideEvent(QHideEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      QWidget* m_parent;
      bool m_has_top;
      bool m_is_menu_shadow;
      bool m_is_visible;

      void follow_parent();
      QSize shadow_size();
  };
}

#endif
