#ifndef SPIRE_FONT_BOX_HPP
#define SPIRE_FONT_BOX_HPP
#include "Spire/Ui/FontFamilyBox.hpp"
#include "Spire/Ui/FontStyleBox.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Represents a font family, style and size. */
  class FontBox : public QWidget {
    public:

      /**
       * Constructs a FontBox using the application's default font.
       * @param parent The parent widget.
       */
      explicit FontBox(QWidget* parent = nullptr);

      /**
       * Constructs a FontBox.
       * @param current The current font that the font box represents.
       * @param parent The parent widget.
       */
      explicit FontBox(std::shared_ptr<ValueModel<QFont>> current,
        QWidget* parent = nullptr);

      /** Returns the current font. */
      const std::shared_ptr<ValueModel<QFont>>& get_current() const;

    protected:
      void showEvent(QShowEvent* event) override;

    private:
      std::shared_ptr<ValueModel<QFont>> m_current;
      FontFamilyBox* m_font_family_box;
      FontStyleBox* m_font_style_box;
      IntegerBox* m_font_size_box;
      boost::signals2::scoped_connection m_font_connection;
      boost::signals2::scoped_connection m_style_connection;
      boost::signals2::scoped_connection m_size_connection;

      void on_current(const QFont& font);
      void on_style_current(const QString& style);
      void on_size_current(const boost::optional<int>& value);
  };
}

#endif
