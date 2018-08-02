#ifndef SPIRE_DIMENSIONS_HPP
#define SPIRE_DIMENSIONS_HPP
#include <QPoint>
#include <QSize>
#include "spire/spire/spire.hpp"

namespace Spire {

  //! The normalized DPI.
  static constexpr auto DPI = 96;

  //! Scales the pixel width at 96 DPI to the system's current DPI.
  /*!
    \param width The width to convert.
    \return The scaled width.
  */
  int scale_width(int width) noexcept;

  //! Scales the pixel height at 96 DPI to the system's current DPI.
  /*!
    \param height The height to convert.
    \return The scaled height.
  */
  int scale_height(int height) noexcept;

  //! Scales a size at 96 DPI to the system's current DPI.
  /*!
    \param width The width to scale.
    \param height The height to scale.
    \return The scaled size.
  */
  QSize scale(int width, int height) noexcept;

  //! Scales a size at 96 DPI to the system's current DPI.
  /*!
    \param size The size to scale.
    \return The scaled size.
  */
  QSize scale(const QSize& size) noexcept;

  //! Translates a point at 96 DPI to the system's current DPI.
  /*!
    \param x The x-coordinate to translate.
    \param y The y-coordinate to translate.
    \return The translated position.
  */
  QPoint translate(int x, int y) noexcept;

  //! Translates a point at 96 DPI to the system's current DPI.
  /*!
    \param point The point to translate.
    \return The translated point.
  */
  QPoint translate(const QPoint& p) noexcept;
}

#endif
