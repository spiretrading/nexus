#ifndef SPIRE_IDENTICON_HPP
#define SPIRE_IDENTICON_HPP
#include <QImage>
#include <QSize>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>

namespace Spire {

  /**
   * Returns a 5x5 identicon representing an account.
   * @param account The account the identicon represents.
   * @param size The size of the rendered image in pixels.
   * @return An image of the identicon at the requested size.
   */
  QImage make_identicon(const Beam::DirectoryEntry& account, QSize size);
}

#endif
