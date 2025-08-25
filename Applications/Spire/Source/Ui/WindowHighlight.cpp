#include "Spire/Ui/WindowHighlight.hpp"
#include <algorithm>
#include <QApplication>
#include <QPainter>
#include <QScreen>
#include <QWindow>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Window.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto OUTLINE_OFFSET = 9;
  const auto HIGHLIGHT_SIZE = 6;
  const auto SNAP_THRESHOLD = 8;

  auto HALF_HIGHLIGHT_WIDTH() {
    static auto width = scale_width(HIGHLIGHT_SIZE / 2);
    return width;
  }

  auto HALF_HIGHLIGHT_HEIGHT() {
    static auto height = scale_height(HIGHLIGHT_SIZE / 2);
    return height;
  }

  auto HORIZONTAL_HIGHLIGHT_OFFSET() {
    static auto offset = scale_width(OUTLINE_OFFSET);
    return offset - HALF_HIGHLIGHT_WIDTH();
  }

  auto VERTICAL_HIGHLIGHT_OFFSET() {
    static auto offset = scale_height(OUTLINE_OFFSET);
    return offset - HALF_HIGHLIGHT_HEIGHT();
  }

  auto HORIZONTAL_SNAP_THRESHOLD() {
    static auto threshold = scale_width(SNAP_THRESHOLD);
    return threshold;
  }

  auto VERTICAL_SNAP_THRESHOLD() {
    static auto threshold = scale_height(SNAP_THRESHOLD);
    return threshold;
  }

  auto contains(const std::vector<Window*>& windows, WId wid) {
    return std::ranges::any_of(windows, [&] (auto window) {
      return window->winId() == wid;
    });
  }

  auto get_z_order_windows() {
    auto windows = std::vector<WId>();
    auto hwnd = GetTopWindow(nullptr);
    while(hwnd) {
      if(IsWindowVisible(hwnd)) {
        windows.push_back(reinterpret_cast<WId>(hwnd));
      }
      hwnd = GetWindow(hwnd, GW_HWNDNEXT);
    }
    return windows;
  }

  auto get_minimized_windows(const std::vector<Window*>& windows) {
    auto minimized_windows = std::unordered_set<WId>();
    for(auto window : windows) {
      auto wid = window->winId();
      if(IsIconic(reinterpret_cast<HWND>(wid))) {
        minimized_windows.insert(wid);
      }
    }
    return minimized_windows;
  }

  void reorder_window(HWND hwnd, HWND insert_after) {
    SetWindowPos(hwnd, insert_after, 0, 0, 0, 0,
      SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
  }

  void raise_window(WId hwnd) {
    auto insert_after_window = [&] {
      if(auto activated_window = QApplication::activeWindow()) {
        return reinterpret_cast<HWND>(activated_window->winId());
      }
      return HWND_TOP;
    }();
    reorder_window(reinterpret_cast<HWND>(hwnd), insert_after_window);
  }

  void raise(const std::vector<WId>& z_order_windows,
      const std::vector<Window*>& targets) {
    if(targets.empty()) {
      return;
    }
    for(auto i = z_order_windows.rbegin(); i != z_order_windows.rend(); ++i) {
      if(contains(targets, *i)) {
        auto hwnd = reinterpret_cast<HWND>(*i);
        if(IsIconic(hwnd)) {
          ShowWindow(hwnd, SW_SHOWNOACTIVATE);
        }
        raise_window(*i);
      }
    }
  }

  void restore(const std::vector<WId>& z_order_windows,
      const std::vector<Window*>& targets,
      const std::unordered_set<WId>& minimized_windows) {
    for(auto i = z_order_windows.begin(); i != z_order_windows.end(); ++i) {
      if(contains(targets, *i)) {
        if(minimized_windows.contains(*i)) {
          ShowWindow(reinterpret_cast<HWND>(*i), SW_SHOWMINNOACTIVE);
        } else {
          auto after_window = [&] {
            if(i == z_order_windows.begin()) {
              return HWND_TOP;
            }
            return reinterpret_cast<HWND>(*(i - 1));
          }();
          reorder_window(reinterpret_cast<HWND>(*i), after_window);
        }
      }
    }
  }

  auto remove_screen_edges(const QScreen& screen, const QPainterPath& path) {
    auto result = QPainterPath();
    auto previous_element = QPainterPath::Element();
    auto screen_geometry = screen.geometry().adjusted(
      HALF_HIGHLIGHT_WIDTH(), HALF_HIGHLIGHT_HEIGHT(),
      -HALF_HIGHLIGHT_WIDTH(), -HALF_HIGHLIGHT_HEIGHT());
    for(auto i = 0; i < path.elementCount(); ++i) {
      auto element = path.elementAt(i);
      if(element.type == QPainterPath::MoveToElement ||
          (element.x == previous_element.x &&
            (element.x <= screen_geometry.left() ||
              element.x >= screen_geometry.right())) ||
          (element.y == previous_element.y &&
            (element.y <= screen_geometry.top() ||
              element.y >= screen_geometry.bottom()))) {
        result.moveTo(element.x, element.y);
      } else {
        result.lineTo(element.x, element.y);
      }
      previous_element = element;
    }
    return result;
  }
}

WindowHighlight::Overlay::Overlay(QScreen& screen)
    : QWidget(nullptr) {
  setWindowFlags(
    Qt::Tool | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_NativeWindow);
  windowHandle()->setScreen(&screen);
}

void WindowHighlight::Overlay::paintEvent(QPaintEvent*) {
  auto painter = QPainter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  auto pen = QPen(QColor(0x7F5EEC), scale_width(HIGHLIGHT_SIZE));
  pen.setJoinStyle(Qt::MiterJoin);
  painter.setPen(pen);
  painter.setBrush(Qt::NoBrush);
  painter.drawPath(m_path);
}

WindowHighlight::WindowHighlight(std::vector<Window*> windows)
    : m_windows(std::move(windows)),
      m_z_order_windows(get_z_order_windows()),
      m_minimized_windows(get_minimized_windows(m_windows)) {
  for(auto window : m_windows) {
    match(*window, Highlighted());
  }
  raise(m_z_order_windows, m_windows);
  update_geometry();
  for(auto& [_, overlay] : m_overlays) {
    raise_window(overlay->winId());
  }
}

WindowHighlight::~WindowHighlight() {
  for(auto window : m_windows) {
    unmatch(*window, Highlighted());
    window->repaint();
  }
  m_overlays.clear();
  restore(m_z_order_windows, m_windows, m_minimized_windows);
}

WindowHighlight::Overlay* WindowHighlight::make_overlay(QScreen* screen) {
  if(auto i = m_overlays.find(screen); i != m_overlays.end()) {
    return i->second.get();
  }
  m_overlays[screen] = std::make_unique<Overlay>(*screen);
  return m_overlays[screen].get();
}

QPainterPath WindowHighlight::make_overlay_path(const QScreen& screen,
    const std::vector<QRect>& rectangles, const QRegion& region) const {
  auto path = QPainterPath();
  for(auto& region_rect : region) {
    path.addRect(region_rect);
  }
  path = path.simplified();
  return remove_screen_edges(screen, path);
}

std::unordered_map<QScreen*, std::vector<QRect>>
    WindowHighlight::get_window_rectangles() const{
  auto rectangles = std::unordered_map<QScreen*, std::vector<QRect>>();
  for(auto window : m_windows) {
    if(!window || window->isHidden()) {
      continue;
    }
    auto window_geometry = window->frameGeometry().adjusted(
      -HORIZONTAL_HIGHLIGHT_OFFSET(), -VERTICAL_HIGHLIGHT_OFFSET(),
      HORIZONTAL_HIGHLIGHT_OFFSET(), VERTICAL_HIGHLIGHT_OFFSET());
    for(auto screen : QGuiApplication::screens()) {
      if(auto intersection = window_geometry.intersected(screen->geometry());
          !intersection.isEmpty()) {
        rectangles[screen].push_back(intersection);
      }
    }
  }
  return rectangles;
}

QRegion WindowHighlight::snap_region(const std::vector<QRect>& rectangles) {
  auto snap = [] (std::vector<int>& edges, int threshold, bool is_ascending) {
    if(is_ascending) {
      std::ranges::sort(edges);
    } else {
      std::ranges::sort(edges, std::greater{});
    }
    auto snapped = std::vector<int>();
    for(auto edge : edges) {
      if(snapped.empty() || std::abs(snapped.back() - edge) > threshold) {
        snapped.push_back(edge);
      }
    }
    return snapped;
  };
  auto find_closest = [] (const std::vector<int>& candidates, int target) {
    return *std::min_element(candidates.begin(), candidates.end(),
      [target] (auto value1, auto value2) {
        return std::abs(value1 - target) < std::abs(value2 - target);
      });
  };
  auto lefts = std::vector<int>();
  auto rights = std::vector<int>();
  auto tops = std::vector<int>();
  auto bottoms = std::vector<int>();
  for(const auto& rectangle : rectangles) {
    lefts.push_back(rectangle.left());
    rights.push_back(rectangle.right());
    tops.push_back(rectangle.top());
    bottoms.push_back(rectangle.bottom());
  }
  auto snapped_lefts = snap(lefts, HORIZONTAL_SNAP_THRESHOLD(), true);
  auto snapped_rights = snap(rights, HORIZONTAL_SNAP_THRESHOLD(), false);
  auto snapped_tops = snap(tops, VERTICAL_SNAP_THRESHOLD(), true);
  auto snapped_bottoms = snap(bottoms, VERTICAL_SNAP_THRESHOLD(), false);
  auto region = QRegion();
  for(auto& rectangle : rectangles) {
    auto left = find_closest(snapped_lefts, rectangle.left());
    auto right = find_closest(snapped_rights, rectangle.right());
    auto top = find_closest(snapped_tops, rectangle.top());
    auto bottom = find_closest(snapped_bottoms, rectangle.bottom());
    if(right > left && bottom > top) {
      region += QRect(QPoint(left, top), QPoint(right, bottom));
    }
  }
  return region;
}

void WindowHighlight::update_geometry() {
  auto window_rectangles = get_window_rectangles();
  for(auto& [screen, rectangles] : window_rectangles) {
    auto region = snap_region(rectangles);
    auto overlay_path = make_overlay_path(*screen, rectangles, region);
    auto rect = region.boundingRect().adjusted(
      -HALF_HIGHLIGHT_WIDTH(), -HALF_HIGHLIGHT_HEIGHT(), HALF_HIGHLIGHT_WIDTH(),
      HALF_HIGHLIGHT_HEIGHT());
    rect = rect.intersected(screen->geometry());
    auto overlay = make_overlay(screen);
    overlay->m_path = overlay_path.translated(-rect.topLeft());
    overlay->setGeometry(rect);
    overlay->show();
    overlay->update();
  }
}
