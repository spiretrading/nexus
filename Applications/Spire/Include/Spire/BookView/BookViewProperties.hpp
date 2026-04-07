#ifndef SPIRE_BOOK_VIEW_PROPERTIES_HPP
#define SPIRE_BOOK_VIEW_PROPERTIES_HPP
#include <array>
#include <filesystem>
#include <vector>
#include <Beam/Serialization/ShuttleArray.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <QColor>
#include <QFont>
#include "Nexus/Definitions/Venue.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/Ui/HighlightBox.hpp"

namespace Spire {

  /** Represents the price level properties in the book view window. */
  struct BookViewLevelProperties {

    /** Specifies the type of fill used for price levels. */
    enum class FillType {

      /** The levels are filled with a linear gradient.*/
      GRADIENT,

      /** The levels are filled using a solid color. */
      SOLID
    };

    /** The font used in the text. */
    QFont m_font;

    /** Whether to show grid lines. */
    bool m_is_grid_enabled;

    /** The fill type for price levels. */
    FillType m_fill_type;

    /** A list of color values used to designate distinct price levels. */
    std::vector<QColor> m_color_scheme;

    /** Returns the default properties. */
    static const BookViewLevelProperties& get_default();

    template<Beam::IsShuttle S>
    void shuttle(S& shuttle, unsigned int version);
  };

  /** Represents the highlight properties in the book view window. */
  struct BookViewHighlightProperties {

    /** Specifies the visibility for the user's orders. */
    enum class OrderVisibility {

      /** The user's orders are not visible. */
      HIDDEN,

      /** The user's orders are visible. */
      VISIBLE,

      /** The user's orders are highlighted. */
      HIGHLIGHTED
    };

    /** Specifies the level of price depth for applying highlights. */
    enum class VenueHighlightLevel {

      /** The highlight applies only to the top price level. */
      TOP,

      /** The highlight applies to all price levels. */
      ALL
    };

    /** Specifies the order state to apply highlight. */
    enum class OrderHighlightState {

      /** The preview order. */
      PREVIEW,

      /** The submitted order. */
      ACTIVE,

      /** The filled order. */
      FILLED,

      /** The canceled order. */
      CANCELED,

      /** The rejected order. */
      REJECTED
    };

    /** The number of the order highlight state. */
    static const auto ORDER_HIGHLIGHT_STATE_COUNT = 5;

    /** Specifies a venue's highlight properties. */
    struct VenueHighlight {

      /** The venue that the highlight applies to. */
      Nexus::Venue m_venue;

      /** The color to highlight the venue with. */
      HighlightColor m_color;

      /** The highlight level. */
      VenueHighlightLevel m_level;

      auto operator <=>(const VenueHighlight&) const = default;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
    };

    /** A list of Highlights for each venue. */
    std::vector<VenueHighlight> m_venue_highlights;

    /** The visibility of the user's orders. */
    OrderVisibility m_order_visibility;

    /** A list of Highlights for the user's orders. */
    std::array<HighlightColor, ORDER_HIGHLIGHT_STATE_COUNT> m_order_highlights;

    /** Returns the default properties. */
    static const BookViewHighlightProperties& get_default();

    template<Beam::IsShuttle S>
    void shuttle(S& shuttle, unsigned int version);
  };

  /** Represents the properties for the book view window. */
  struct BookViewProperties {

    /** The properties related to the price level. */
    BookViewLevelProperties m_level_properties;

    /** The properties related to the highlight. */
    BookViewHighlightProperties m_highlight_properties;

    /** Returns the default properties. */
    static const BookViewProperties& get_default();

    template<Beam::IsShuttle S>
    void shuttle(S& shuttle, unsigned int version);
  };

  /** A ValueModel over a BookViewProperties. */
  using BookViewPropertiesModel = ValueModel<BookViewProperties>;

  /** A LocalValueModel over a BookViewProperties. */
  using LocalBookViewPropertiesModel = LocalValueModel<BookViewProperties>;

  /** Returns the HighlightColor associated with an OrderHighlightState. */
  const HighlightColor& get_highlight(const BookViewProperties& properties,
    BookViewHighlightProperties::OrderHighlightState state);

  /** Returns the text representation of a OrderVisibility. */
  const QString& to_text(
    BookViewHighlightProperties::OrderVisibility visibility);

  /** Returns the text representation of a VenueHighlightLevel. */
  const QString& to_text(
    BookViewHighlightProperties::VenueHighlightLevel level);

  /** Returns the text representation of a OrderHighlightState. */
  const QString& to_text(
    BookViewHighlightProperties::OrderHighlightState state);

  /**
   * Loads BookViewProperties from a file.
   * @param path The path to the file to load.
   * @return The properties loaded from the given <i>path</i>.
   */
  BookViewProperties load_book_view_properties(
    const std::filesystem::path& path);

  /**
   * Saves BookViewProperties to a file.
   * @param properties The properties to save.
   * @param path The path to the save the properties to.
   */
  void save_book_view_properties(
    const BookViewProperties& properties, const std::filesystem::path& path);

  template<Beam::IsShuttle S>
  void BookViewLevelProperties::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("font", m_font);
    shuttle.shuttle("is_grid_enabled", m_is_grid_enabled);
    shuttle.shuttle("fill_type", m_fill_type);
    shuttle.shuttle("color_scheme", m_color_scheme);
  }

  template<Beam::IsShuttle S>
  void BookViewHighlightProperties::VenueHighlight::shuttle(
      S& shuttle, unsigned int version) {
    shuttle.shuttle("venue", m_venue);
    shuttle.shuttle("color", m_color);
    shuttle.shuttle("level", m_level);
  }

  template<Beam::IsShuttle S>
  void BookViewHighlightProperties::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("venue_highlights", m_venue_highlights);
    shuttle.shuttle("order_visibility", m_order_visibility);
    shuttle.shuttle("order_highlights", m_order_highlights);
  }

  template<Beam::IsShuttle S>
  void BookViewProperties::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("level_properties", m_level_properties);
    shuttle.shuttle("highlight_properties", m_highlight_properties);
  }
}

#endif
