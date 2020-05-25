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
      explicit InputFieldEditor(QString initial_value,
        std::vector<QString> items, QWidget* parent = nullptr);

      //! Returns the selected item, or the initial item if an invalid input
      //! was entered.
      const QString& get_item() const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      QString m_selected_item;
      std::vector<QString> m_items;
      DropDownMenuList* m_menu_list;

      void move_menu_list();
      void on_item_clicked(const QString& item);
      void on_item_committed(const QString& text);
      void on_text_changed(const QString& text);
  };
}

#endif
