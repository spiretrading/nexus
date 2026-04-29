#ifndef SPIRE_SCOPE_LIST_ITEM_HPP
#define SPIRE_SCOPE_LIST_ITEM_HPP
#include <QWidget>
#include "Nexus/Definitions/Scope.hpp"

namespace Spire {
  class Icon;
  class TextBox;

  /**
   * Represents a scope item which can be a ticker, a venue or a country in a
   * ScopeBox.
   */
  class ScopeListItem : public QWidget {
    public:

      /**
       * Constructs a ScopeListItem.
       * @param scope The scope which can be a ticker, a venue or a country.
       * @param parent The parent widget.
       */
      explicit ScopeListItem(Nexus::Scope scope, QWidget* parent = nullptr);

      /** Returns the scope. */
      const Nexus::Scope& get_scope() const;

    private:
      enum class Type : std::uint8_t {
        NONE,
        TICKER,
        VENUE,
        COUNTRY
      };
      Nexus::Scope m_scope;
      Type m_type;

      Type get_type() const;
      TextBox* make_value_label() const;
      Icon* make_type_icon() const;
  };
}

#endif
