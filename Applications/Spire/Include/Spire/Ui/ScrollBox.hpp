#ifndef SPIRE_SCROLL_BOX_HPP
#define SPIRE_SCROLL_BOX_HPP
#include "Spire/Ui/Ui.hpp"
#include "Spire/Styles/StyledWidget.hpp"

namespace Spire {
  class ScrollBar : public Styles::StyledWidget {
    public:
      enum Orientation {
        VERTICAL,
        HORIZONTAL
      };

      explicit ScrollBar(Orientation orientation, QWidget* parent = nullptr);

      QSize sizeHint() const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      Orientation m_orientation;
      Box* m_thumb;
      Box* m_track;
      bool m_is_dragging;
      int m_drag_position;
      int m_thumb_start;
  };
}

#endif
