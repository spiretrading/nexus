#ifndef SPIRE_INPUT_FIELD_EDITOR_HPP
#define SPIRE_INPUT_FIELD_EDITOR_HPP
#include <QLineEdit>
#include "Spire/Ui/DropDownMenuList.hpp"

namespace Spire {

  class InputFieldEditor : public QLineEdit {
    public:

      explicit InputFieldEditor(std::vector<QString> items,
        int list_width, QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      DropDownMenuList* m_menu_list;
      int m_list_width;

      void move_menu_list();
      void on_clicked();
      void on_item_selected(const QString& text);
  };
}

#endif
