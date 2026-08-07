#ifndef SPIRE_WINDOW_HIGHLIGHT_HPP
#define SPIRE_WINDOW_HIGHLIGHT_HPP
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <QPainterPath>
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Displays highlight borders around a set of tracked windows. */
  class WindowHighlight {
    public:

      /**
       * Constructs a WindowHighlight.
       * @param current The list of tracked windows.
       */
      explicit WindowHighlight(std::shared_ptr<ListModel<Window*>> current);

      ~WindowHighlight();

      /** Returns the list of tracked windows. */
      const std::shared_ptr<ListModel<Window*>>& get_current() const;

    private:
      struct Overlay;
      std::shared_ptr<ListModel<Window*>> m_current;
      std::vector<WId> m_z_order_windows;
      std::unordered_set<WId> m_minimized_windows;
      std::unordered_map<QScreen*, std::unique_ptr<Overlay>> m_overlays;
      std::unordered_map<QScreen*, QRect> m_overlay_bounds;
      int m_transaction_depth;
      boost::signals2::scoped_connection m_operation_connection;

      QPainterPath make_overlay_path(const QScreen& screen,
        const std::vector<QRect>& rectangles, const QRegion& region) const;
      std::unordered_map<QScreen*, std::vector<QRect>>
        get_window_rectangles() const;
      QRegion snap_region(const std::vector<QRect>& rectangles);
      void update_geometry();
      void refresh();
      void on_operation(const ListModel<Window*>::Operation& operation);
  };
}

#endif
