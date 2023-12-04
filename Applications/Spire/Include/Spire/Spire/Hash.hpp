#ifndef SPIRE_HASH_HPP
#define SPIRE_HASH_HPP
#include <QColor>
#include <QFlags>
#include <QFont>
#include <QImage>

namespace std {
  template<>
  struct hash<QColor> {
    std::size_t operator ()(const QColor& color) const;
  };

  template<typename E>
  struct hash<QFlags<E>> {
    std::size_t operator ()(const QFlags<E>& flags) const {
      return static_cast<int>(flags);
    }
  };

  template<>
  struct hash<QFont> {
    std::size_t operator ()(const QFont& font) const;
  };

  template<>
  struct hash<QImage> {
    std::size_t operator ()(const QImage& image) const;
  };
}

#endif
