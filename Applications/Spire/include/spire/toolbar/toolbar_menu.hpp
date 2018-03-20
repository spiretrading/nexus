#ifndef SPIRE_TOOLBAR_MENU_HPP
#define SPIRE_TOOLBAR_MENU_HPP
#include <unordered_map>
#include <QMenu>
#include <QPushButton>
#include <QResizeEvent>
#include <QString>
#include <QWidgetAction>
#include "spire/toolbar/toolbar.hpp"

namespace spire {

  //! \brief Provides a drop-down menu with a title.
  class toolbar_menu : public QPushButton {
    public:

      using item_selected_signal = signal<void (int index)>;

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
      void add(const QString& text);

      //! Adds an item to the menu with an icon.
      /*!
        \param text The text string for the item.
        \param icon_path Resource path or absolute path to the icon.
      */
      void add(const QString& text, const QString& icon_path);

      void remove(int index);

      boost::signals2::connection connect_item_selected_signal(
        const item_selected_signal::slot_type& slot) const;

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable item_selected_signal m_item_selected_signal;
      QMenu* m_items;
      QWidgetAction* m_empty_item;
      std::unordered_map<QAction*, int> m_action_to_index;
      bool m_default_style;

      void remove_empty_item();
      void set_stylesheet(int padding_left);
      void set_empty_menu_stylesheet();
      void set_default_menu_stylesheet(int padding_left);
      void on_triggered(QAction* action);
  };
}

#endif
