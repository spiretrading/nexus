#ifndef SPIRE_COMBO_BOX_EDITOR_HPP
#define SPIRE_COMBO_BOX_EDITOR_HPP
#include <QLineEdit>
#include "Spire/Ui/StaticDropDownMenu.hpp"

namespace Spire {

  //! Represents a combo box editor used in a table cell.
  class ComboBoxEditor : public QLineEdit {
  
    public:

      ComboBoxEditor(StaticDropDownMenu* menu, QWidget* parent = nullptr);

      Qt::Key get_last_key() const;

      QVariant get_value() const;

      void set_value(const QVariant& value);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      StaticDropDownMenu* m_menu;
      Qt::Key m_last_key;
  };
}

#endif
