#ifndef SPIRE_STYLES_STYLE_SHEET_MAP_HPP
#define SPIRE_STYLES_STYLE_SHEET_MAP_HPP
#include <functional>
#include <unordered_map>
#include <utility>
#include <QColor>
#include <QString>
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Stores a set of Qt stylesheet properties. */
  class StyleSheetMap {
    public:

      /**
       * Constructs an empty StyleSheetMap.
       * @param commit The function used to commit property updates.
       */
      StyleSheetMap(std::function<void ()> commit);

      /**
       * Writes out the stylesheet to a string.
       * @param stylesheet The string to write the stylesheet to.
       */
      void write(QString& stylesheet) const;

      /**
       * Buffers multiple property set operations so that they are committed
       * together.
       * @param f The function that is called to perform the set operations.
       */
      template<typename F>
      void buffer(F&& f);

      /** Sets a QColor style property. */
      void set(const QString& property, QColor);

      /** Sets an int style property. */
      void set(const QString& property, int value);

      /** Clears all properties, reverting to an empty set. */
      void clear();

    private:
      std::function<void ()> m_commit;
      bool m_is_buffering;
      std::unordered_map<QString, QString> m_properties;
  };

  template<typename F>
  void StyleSheetMap::buffer(F&& f) {
    m_is_buffering = true;
    std::forward<F>(f)();
    m_is_buffering = false;
    m_commit();
  }
}

#endif
