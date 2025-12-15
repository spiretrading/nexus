#ifndef SPIRE_COLOR_SWATCH_HPP
#define SPIRE_COLOR_SWATCH_HPP
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/ColorBox.hpp"

namespace Spire {
namespace Styles {

  /** Selects the highlighted widget. */
  using Highlighted = StateSelector<void, struct HighlightedSelectorTag>;
}

  /** Represents a color in the color palette. */
  class ColorSwatch : public QWidget {
    public:
      
      /**
       * Constructs a ColorSwatch with a LocalValueModel.
       * @param parent The parent widget.
       */
      explicit ColorSwatch(QWidget* parent = nullptr);

      /**
       * Constructs a ColorSwatch.
       * @param current The current color model.
       * @param parent The parent widget.
       */
      explicit ColorSwatch(std::shared_ptr<ColorModel> current,
        QWidget* parent = nullptr);

      /** Returns the current color model. */
      const std::shared_ptr<ColorModel>& get_current() const;

      /** Returns <code>true</code> iff this swatch is highlighted. */
      bool is_highlighted() const;

      /** Sets whether the swatch is highlighted. */
      void set_highlighted(bool highlighted);

    private:
      std::shared_ptr<ColorModel> m_current;
      bool m_is_highlighted;
      boost::signals2::scoped_connection m_current_connection;

      void on_current(const QColor& highlight);
  };
}

#endif
