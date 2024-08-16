#ifndef SPIRE_FONTSELECTIONWIDGET_HPP
#define SPIRE_FONTSELECTIONWIDGET_HPP
#include <boost/signals2/connection.hpp>
#include <boost/signals2/signal.hpp>
#include <QFont>
#include <QWidget>
#include "Spire/UI/UI.hpp"

class Ui_FontSelectionWidget;

namespace Spire {
namespace UI {

  /*! \class FontSelectionWidget
      \brief Standard widget used to select a font.
   */
  class FontSelectionWidget : public QWidget {
    public:

      //! Signals a change in the selected font.
      /*!
        \param oldFont The previously selected font.
        \param newFont The newly selected font.
      */
      typedef boost::signals2::signal<void (const QFont& oldFont,
        const QFont& newFont)> FontChangedSignal;

      //! Constructs a FontSelectionWidget.
      /*!
        \param parent The parent widget.
        \param flags The flags passed to the <i>parent</i> widget.
      */
      FontSelectionWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~FontSelectionWidget();

      //! Returns the selected font.
      const QFont& GetFont() const;

      //! Sets the font to display.
      /*!
        \param font The font to display.
      */
      void SetFont(const QFont& font);

      //! Connects a slot to the FontChangedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectFontChangedSignal(
        const FontChangedSignal::slot_type& slot) const;

    private:
      std::unique_ptr<Ui_FontSelectionWidget> m_ui;
      QFont m_font;
      bool m_ignoreFontChange;
      mutable FontChangedSignal m_fontChangedSignal;

      void OnAdvancedFontButtonClicked();
      void OnFontChanged(const QFont& font);
      void OnFontSizeChanged(int value);
  };
}
}

#endif
