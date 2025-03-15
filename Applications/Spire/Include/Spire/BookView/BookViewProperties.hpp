#ifndef SPIRE_BOOK_VIEW_PROPERTIES_HPP
#define SPIRE_BOOK_VIEW_PROPERTIES_HPP
#include <array>
#include <filesystem>
#include <vector>
#include <Beam/Serialization/ShuttleArray.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <QColor>
#include <QFont>
#include "Nexus/Definitions/Market.hpp"
#include "Spire/BookView/BookView.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/Ui/HighlightBox.hpp"

namespace Spire {

  /** A ValueModel over a BookViewProperties. */
  using BookViewPropertiesModel = ValueModel<BookViewProperties>;

  /** A LocalValueModel over a BookViewProperties. */
  using LocalBookViewPropertiesModel = LocalValueModel<BookViewProperties>;

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

    template<typename Shuttler>
    void Shuttle(Shuttler& shuttle, unsigned int version);
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
    enum class MarketHighlightLevel {

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

    /** Specifies a market's highlight properties. */
    struct MarketHighlight {

      /** The market that the highlight applies to. */
      Nexus::MarketCode m_market;

      /** The color to highlight the market with. */
      HighlightColor m_color;

      /** The highlight level. */
      MarketHighlightLevel m_level;

      auto operator <=>(const MarketHighlight&) const = default;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
    };

    /** A list of Highlights for each market. */
    std::vector<MarketHighlight> m_market_highlights;

    /** The visibility of the user's orders. */
    OrderVisibility m_order_visibility;

    /** A list of Highlights for the user's orders. */
    std::array<HighlightColor, ORDER_HIGHLIGHT_STATE_COUNT> m_order_highlights;

    /** Returns the default properties. */
    static const BookViewHighlightProperties& get_default();

    template<typename Shuttler>
    void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  /** Represents the properties for the book view window. */
  struct BookViewProperties {

    /** The properties related to the price level. */
    BookViewLevelProperties m_level_properties;

    /** The properties related to the highlight. */
    BookViewHighlightProperties m_highlight_properties;

    /** Returns the default properties. */
    static const BookViewProperties& get_default();

    template<typename Shuttler>
    void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  /** Returns the text representation of a OrderVisibility. */
  const QString& to_text(
    BookViewHighlightProperties::OrderVisibility visibility);

  /** Returns the text representation of a MarketHighlightLevel. */
  const QString& to_text(
    BookViewHighlightProperties::MarketHighlightLevel level);

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

  template<typename Shuttler>
  void BookViewLevelProperties::Shuttle(
      Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("font", m_font);
    shuttle.Shuttle("is_grid_enabled", m_is_grid_enabled);
    shuttle.Shuttle("fill_type", m_fill_type);
    shuttle.Shuttle("color_scheme", m_color_scheme);
  }

  template<typename Shuttler>
  void BookViewHighlightProperties::MarketHighlight::Shuttle(
      Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("market", m_market);
    shuttle.Shuttle("color", m_color);
    shuttle.Shuttle("level", m_level);
  }

  template<typename Shuttler>
  void BookViewHighlightProperties::Shuttle(
      Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("market_highlights", m_market_highlights);
    shuttle.Shuttle("order_visibility", m_order_visibility);
    shuttle.Shuttle("order_highlights", m_order_highlights);
  }

  template<typename Shuttler>
  void BookViewProperties::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("level_properties", m_level_properties);
    shuttle.Shuttle("highlight_properties", m_highlight_properties);
  }
}

#endif
