#ifndef SPIRE_CHECK_BOX_HPP
#define SPIRE_CHECK_BOX_HPP
#include <QWidget>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

namespace Spire {
  class Icon;
  class TextBox;

namespace Styles {

  /** Selects a checked CheckBox. */
  using Checked = StateSelector<void, struct CheckedSelectorTag>;
}

  /** A ValueModel over a bool. */
  using BooleanModel = ValueModel<bool>;

  /** A ValueModel over an optional bool. */
  using OptionalBooleanModel = ValueModel<boost::optional<bool>>;

  /** A LocalValueModel over a bool. */
  using LocalBooleanModel = LocalValueModel<bool>;

  /** A LocalValueModel over an optional bool. */
  using LocalOptionalBooleanModel = LocalValueModel<boost::optional<bool>>;

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
       * @param current The current value model.
       * @param parent The parent widget.
       */
      explicit CheckBox(
        std::shared_ptr<BooleanModel> current, QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<BooleanModel>& get_current() const;

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

    protected:
      void changeEvent(QEvent* event) override;

    private:
      std::shared_ptr<BooleanModel> m_current;
      bool m_is_read_only;
      Icon* m_check;
      TextBox* m_label;
      boost::signals2::scoped_connection m_connection;

      void on_current(bool current);
      void on_layout_direction(Qt::LayoutDirection direction);
  };

  /**
   * Returns a newly constructed RadioButton.
   * @param parent The parent widget.
   */
  CheckBox* make_radio_button(QWidget* parent = nullptr);
}

#endif
