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

    protected:
      void changeEvent(QEvent* event) override;

    private:
      std::shared_ptr<BooleanModel> m_model;
      bool m_is_read_only;
      Icon* m_check;
      TextBox* m_label;

      void on_checked(bool is_checked);
  };
}

#endif
