#ifndef SPIRE_REGION_LIST_ITEM_HPP
#define SPIRE_REGION_LIST_ITEM_HPP
#include <QWidget>
#include "Nexus/Definitions/Region.hpp"

namespace Spire {
  class Icon;
  class TextBox;

  /**
   * Represents a region item which can be a security, a venue or a country
   * in a RegionBox.
   */
  class RegionListItem : public QWidget {
    public:

      /**
       * Constructs a RegionListItem.
       * @param region The region which can be a security, a venue or a country.
       * @param parent The parent widget.
       */
      explicit RegionListItem(Nexus::Region region, QWidget* parent = nullptr);

      /** Returns the region. */
      const Nexus::Region& get_region() const;

    private:
      enum class Type : std::uint8_t {
        NONE,
        SECURITY,
        VENUE,
        COUNTRY
      };
      Nexus::Region m_region;
      Type m_type;

      Type get_type() const;
      TextBox* make_value_label() const;
      Icon* make_type_icon() const;
  };
}

#endif
