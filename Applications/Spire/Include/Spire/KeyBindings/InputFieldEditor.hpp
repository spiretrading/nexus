#ifndef SPIRE_INPUT_FIELD_EDITOR_HPP
#define SPIRE_INPUT_FIELD_EDITOR_HPP
#include <QLineEdit>
#include "Spire/Ui/DropDownMenuList.hpp"

namespace Spire {

  class InputFieldEditor : public QLineEdit {
    public:

      explicit InputFieldEditor(QString initial_value,
        std::vector<QString> items, QWidget* parent = nullptr);

      const QString& get_item() const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      QString m_selected_item;
      std::vector<QString> m_items;
      DropDownMenuList* m_menu_list;

      void move_menu_list();
      void on_item_selected(const QString& text);
      void on_text_changed(const QString& text);
  };
}

#endif
