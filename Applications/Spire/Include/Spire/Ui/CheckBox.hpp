#ifndef SPIRE_CHECKBOX_HPP
#define SPIRE_CHECKBOX_HPP
#include <QWidget>
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/ValueModel.hpp"

namespace Spire {
namespace Styles {

  /** Selects a checked CheckBox. */
  using Checked = StateSelector<void, struct CheckedTag>;
}

  /** Represents a Spire-styled CheckBox. */
  class CheckBox : public QWidget {
    public:

      /**
      * Signals that the checked state has changed.
      * @param is_checked True iff the CheckBox is checked.
      */
      using CheckedSignal = Signal<void (bool is_checked)>;

      /**
       * Constructs a CheckBox using a LocalBooleanModel with an initial value
       * of false.
       * @param parent The parent widget.
       */
      explicit CheckBox(QWidget* parent = nullptr);
  
      /**
       * Constructs a CheckBox.
       * @param model The check state's model.
       * @param parent The parent widget.
       */
      explicit CheckBox(std::shared_ptr<BooleanModel> model,
        QWidget* parent = nullptr);

      /** Returns the model. */
      const std::shared_ptr<BooleanModel>& get_model() const;

      /**
       * Sets the text of the label.
       * @param label The label's text.
       */
      void set_label(const QString& label);

      /**
       * Sets the read-only state of the CheckBox.
       * @param is_read_only Sets the CheckBox to read-only iff is_read_only is
       *                     true.
       */
      void set_read_only(bool is_read_only);

      QSize sizeHint() const override;

      /** Connects a slot to the checked signal. */
      boost::signals2::connection connect_checked_signal(
        const CheckedSignal::slot_type& slot) const;

    protected:
      void changeEvent(QEvent* event) override;

    private:
      mutable CheckedSignal m_checked_signal;
      std::shared_ptr<BooleanModel> m_model;
      bool m_is_read_only;
      Icon* m_check;
      TextBox* m_label;

      void on_checked(bool is_checked);
  };
}

#endif
