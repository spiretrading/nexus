#ifndef SPIRE_INPUT_FIELD_EDITOR_HPP
#define SPIRE_INPUT_FIELD_EDITOR_HPP
#include <QLineEdit>
#include "Spire/Ui/DropDownMenuList.hpp"

namespace Spire {

  //! Represents a line edit with a dynamic drop down list that displays a list
  //! of auto-complete suggestions.
  class InputFieldEditor : public QLineEdit {
    public:

      //! Constructs an InputFieldEditor.
      /*
        \param initial_value The default value of the input field.
        \param items The list of valid inputs.
        \param parent The parent widget.
      */
      explicit InputFieldEditor(QVariant initial_value,
        std::vector<QVariant> items, QWidget* parent = nullptr);

      //! Returns the selected item, or the initial item if an invalid input
      //! was entered.
      const QVariant& get_item() const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      QVariant m_selected_item;
      QVariant m_highlighted_item;
      std::vector<QVariant> m_items;
      DropDownMenuList* m_menu_list;

      std::vector<DropDownMenuItem*> create_widget_items(
        const std::vector<QVariant>& items, const QString& filter_text);
      void move_menu_list();
      void on_item_clicked(const QVariant& item);
      void on_item_committed(const QVariant& text);
      void on_item_highlighted(const QVariant& item);
      void on_text_edited(const QString& text);
  };
}

#endif
