#ifndef SPIRE_EDITABLE_BOX_HPP
#define SPIRE_EDITABLE_BOX_HPP
#include <QWidget>
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Encapsulates an AnyInputBox which can flip read-only when focus changes.
   */
  class EditableBox : public QWidget {
    public:

      /**
       * Constructs a EditableBox.
       * @param input_box The AnyInputBox to encapsulate.
       * @param parent The parent widget.
       */
      explicit EditableBox(AnyInputBox& input_box, QWidget* parent = nullptr);

      /** Returns the AnyInputBox. */
      const AnyInputBox& get_input_box() const;

      /** Returns the AnyInputBox. */
      AnyInputBox& get_input_box();

    protected:
      //bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      AnyInputBox* m_input_box;
      FocusObserver m_focus_observer;

      void on_focus(FocusObserver::State state);
  };
}

#endif
