#ifndef SPIRE_UI_HPP
#define SPIRE_UI_HPP
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <QImage>
#include <QPropertyAnimation>
#include <QRect>
#include <QSize>
#include <QString>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Fades a window in by animating its window opacity. The returned animation
   * is deleted on completion.
   * @param target The window to fade in or out. Used as the parent for the
   *               returned QPropertyAnimation.
   * @param reverse True iff the animation should be reversed (fade out).
   * @param fade_speed_ms The fade speed, in milliseconds.
   * @returns The animation assigned to the target.
   */
  QPropertyAnimation* fade_window(
    QObject* target, bool reverse, boost::posix_time::time_duration fade_speed);

  /**
   * Find the focus proxy that is at the bottom of the proxy chain of a widget.
   * @param widget The widget with a focus proxy.
   * @returns The focus proxy widget.
   */
  QWidget* find_focus_proxy(QWidget& widget);

  /**
   * Builds a QImage from an SVG resource where the size of the SVG is equal
   * to the image as a whole.
   * @param path The path to the SVG resource.
   * @param size The size of the image to render.
   * @return An image rendered from the SVG resource to the specified size.
   */
  QImage image_from_svg(const QString& path, const QSize& size);

  /**
   * Builds a QImage from an SVG resource where the size of the SVG is embedded
   * within the image.
   * @param path The path to the SVG resource.
   * @param size The size of the image to render.
   * @param box The location and size to render the SVG within the image.
   * @return An image rendered from the SVG resource to the specified
   *         dimensions.
   */
  QImage image_from_svg(
    const QString& path, const QSize& size, const QRect& box);

  /**
   * Invalidates the layout of all descendants of the widget.
   * @param widget The widget whose descendants' layout will be invalidated.
   */
  void invalidate_descendant_layouts(QWidget& widget);

  /**
   * Gets the width of a character.
   * @param font The font used to calculate the character width.
   */
  int get_character_width(const QFont& font);
}

#endif
