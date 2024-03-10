#ifndef SPIRE_HASH_QT_TYPES_HPP
#define SPIRE_HASH_QT_TYPES_HPP
#include <unordered_map>

class QKeySequence;

namespace std {
  template<>
  struct hash<QKeySequence> {
    std::size_t operator ()(const QKeySequence& value) const;
  };
}

#endif
