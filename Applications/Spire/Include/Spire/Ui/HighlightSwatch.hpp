#ifndef SPIRE_HIGHLIGHT_SWATCH_HPP
#define SPIRE_HIGHLIGHT_SWATCH_HPP
#include <QWidget>
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /** Displays a Label to preview a highlight. */
  class HighlightSwatch : public QWidget {
    public:
      
      /** The Highlight represented by the HighlightSwatch. */
      struct Highlight {

        /** The background color for the Highlight. */
        QColor m_background_color;

        /** The text color for the Highlight. */
        QColor m_text_color;

        auto operator <=>(const Highlight&) const = default;
      };

      /**
       * Constructs a HighlightSwatch with a LocalValueModel.
       * @param parent The parent widget.
       */
      explicit HighlightSwatch(QWidget* parent = nullptr);

      /**
       * Constructs a HighlightSwatch.
       * @param current The current value model.
       * @param parent The parent widget.
       */
      explicit HighlightSwatch(std::shared_ptr<ValueModel<Highlight>> current,
        QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<ValueModel<Highlight>>& get_current() const;

    private:
      std::shared_ptr<ValueModel<Highlight>> m_current;
      boost::signals2::scoped_connection m_current_connection;

      void on_current(const Highlight& highlight);
  };
}

namespace Beam {
  template<>
  struct Shuttle<Spire::HighlightSwatch::Highlight> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Spire::HighlightSwatch::Highlight& value,
        unsigned int version) const {
      shuttle.shuttle("background_color", value.m_background_color);
      shuttle.shuttle("text_color", value.m_text_color);
    }
  };
}

#endif
