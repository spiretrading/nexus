#ifndef SPIRE_DROP_SHADOW_HPP
#define SPIRE_DROP_SHADOW_HPP
#include <QAbstractNativeEventFilter>
#include <QWidget>

namespace Spire {

  /** Paints a drop shadow around its parent widget. */
  class DropShadow : public QWidget, public QAbstractNativeEventFilter {
    public:

      /**
       * Constructs a drop shadow.
       * @param parent The parent to add the drop shadow to.
       */
      explicit DropShadow(QWidget* parent);

      /**
       * Constructs a drop shadow.
       * @param has_top <code>true</code> iff the top gradient is drawn.
       * @param parent The parent to add the drop shadow to.
       */
      DropShadow(bool has_top, QWidget* parent);

      /**
       * Constructs a drop shadow.
       * @param is_menu_shadow <code>true</code> iff the drop shadow is being
       *                       used with a menu.
       * @param has_top <code>true</code> iff the top gradient is drawn.
       * @param parent The parent to add the drop shadow to.
       */
      DropShadow(bool is_menu_shadow, bool has_top, QWidget* parent);

    protected:
      bool event(QEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      void hideEvent(QHideEvent* event) override;
      bool nativeEventFilter(
        const QByteArray& event_type, void* message, long* result) override;
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
