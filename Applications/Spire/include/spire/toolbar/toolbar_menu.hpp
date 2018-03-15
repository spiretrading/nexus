#ifndef SPIRE_TOOLBAR_MENU_HPP
#define SPIRE_TOOLBAR_MENU_HPP
#include <QMenu>
#include <QPushButton>
#include <QResizeEvent>
#include <QString>

namespace spire {

  //! \brief Provides a drop-down menu with a title.
  class toolbar_menu : public QPushButton {
    public:

      //! Constructs an empty toolbar_menu.
      /*!
        \param title The text on the top-level button.
        \param parent The parent to the toolbar_menu.
      */
      toolbar_menu(const QString& title, QWidget* parent = nullptr);

      //! Constructs an empty toolbar_menu.
      /*!
        \param text The text string for the item.
      */
      void add_item(const QString& text);

      //! Adds an item to the menu with an icon.
      /*!
        \param text The text string for the item.
        \param icon Resource path or absolute path to the icon.
      */
      void add_item(const QString& text, const QString& icon);

    protected:
      void resizeEvent(QResizeEvent* event);

    private:
      QMenu* m_items;
      bool m_default_style;

      void set_stylesheet(int padding_left);
  };
}

#endif
