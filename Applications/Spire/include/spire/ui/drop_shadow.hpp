#ifndef SPIRE_DROP_SHADOW
#define SPIRE_DROP_SHADOW
#include <QWidget>
#include "spire/ui/drop_shadow.hpp"

namespace spire {
  //! Creates a drop shadow around its parent widget.
  class drop_shadow : public QWidget {
    public:

      //! Constructs a drop shadow with the specified margins between the
      //! parent widget and the outside of the drop shadow.
      /*!
        \param margins The margins of the drop shadow.
        \param parent The parent to the drop shadow.
      */
      drop_shadow(const QMargins& margins, QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      QMargins m_margins;
      QWidget* m_inner_widget;
      bool m_is_shown;

      void move_to_parent();
  };
}

#endif
