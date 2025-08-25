#ifndef SPIRE_WINDOW_HIGHLIGHT_HPP
#define SPIRE_WINDOW_HIGHLIGHT_HPP
#include <unordered_map>
#include <unordered_set>
#include <QPainterPath>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays highlight borders around a set of tracked windows. */
  class WindowHighlight {
    public:

      /**
       * Constructs a WindowHighlight.
       * @param windows A list of tracked windows.
       */
      explicit WindowHighlight(std::vector<Window*> windows);

      ~WindowHighlight();

    private:
      struct Overlay : QWidget {
        QPainterPath m_path;

        explicit Overlay(QScreen& screen);
        void paintEvent(QPaintEvent*) override;
      };
      std::vector<Window*> m_windows;
      std::vector<WId> m_z_order_windows;
      std::unordered_set<WId> m_minimized_windows;
      std::unordered_map<QScreen*, std::unique_ptr<Overlay>> m_overlays;

      QPainterPath make_overlay_path(const QScreen& screen,
        const std::vector<QRect>& rectangles, const QRegion& region) const;
      std::unordered_map<QScreen*, std::vector<QRect>>
        get_window_rectangles() const;
      QRegion snap_region(const std::vector<QRect>& rectangles);
      void update_geometry();
  };
}

#endif
