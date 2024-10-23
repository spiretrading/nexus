#ifndef SPIRE_SECURITY_DIALOG_HPP
#define SPIRE_SECURITY_DIALOG_HPP
#include <QWidget>
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/SecurityBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Displays a dialog containing a SecurityBox to allow a user to enter a
   * security.
   */
  class SecurityDialog : public QWidget {
    public:
      using SubmitSignal = SecurityBox::SubmitSignal;

      /**
       * Constructs a SecurityDialog.
       * @param securities The set of securities that can be queried.
       * @param parent The parent widget.
       */
      explicit SecurityDialog(std::shared_ptr<SecurityQueryModel> securities,
        QWidget* parent = nullptr);

      /**
       * Constructs a SecurityDialog.
       * @param securities The set of securities that can be queried.
       * @param current The current security to display.
       * @param parent The parent widget.
       */
      explicit SecurityDialog(std::shared_ptr<SecurityQueryModel> securities,
        std::shared_ptr<SecurityModel> current, QWidget* parent = nullptr);

      /** Returns the set of securities that can be queried. */
      const std::shared_ptr<SecurityQueryModel>& get_securities() const;

      /** Returns the current security. */
      const std::shared_ptr<SecurityModel>& get_current() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      SecurityBox* m_security_box;
      OverlayPanel* m_panel;
      QWidget* m_input_box;
  };
}

#endif
