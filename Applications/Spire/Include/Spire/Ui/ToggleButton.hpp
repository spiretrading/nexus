#ifndef SPIRE_TOGGLE_BUTTON_HPP
#define SPIRE_TOGGLE_BUTTON_HPP
#include <QAbstractButton>
#include "Spire/Ui/IconButton.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a button using an icon that can be checked.
  class ToggleButton : public QAbstractButton {
    public:

      //! Constructs a ToggleButton.
      /*!
        \param icon The button's icon.
        \param parent The parent QWidget to the ToggleButton.
      */
      explicit ToggleButton(QImage icon, QWidget* parent = nullptr);

    protected:
      void changeEvent(QEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
      QSize sizeHint() const override;

    private:
      QImage m_icon;
      IconButton* m_icon_button;

      void update_button();
      void update_button(bool enabled);
  };
}

#endif
