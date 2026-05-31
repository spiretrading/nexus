#ifndef SPIRE_NEW_BLOTTER_FORM_HPP
#define SPIRE_NEW_BLOTTER_FORM_HPP
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <QWidget>
#include "Spire/Ui/AccountBox.hpp"
#include "Spire/Ui/TextBox.hpp"

namespace Spire {
  class Button;
  class OverlayPanel;

  /** Displays a form to create a new blotter. */
  class NewBlotterForm : public QWidget {
    public:

      /**
       * Signals a submission.
       * @param name The name of the new blotter.
       * @param account The account the blotter is for.
       */
      using SubmitSignal =
        Signal<void (const QString& name, const Beam::DirectoryEntry& account)>;

      /**
       * Constructs a NewBlotterForm.
       * @param account The default account.
       * @param accounts The query model for looking up accounts.
       * @param is_account_selectable Whether the account field is shown.
       * @param parent The parent widget.
       */
      NewBlotterForm(Beam::DirectoryEntry account,
        std::shared_ptr<AccountQueryModel> accounts,
        bool is_account_selectable, QWidget& parent);

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection
        connect_submit_signal(const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      Beam::DirectoryEntry m_account;
      std::shared_ptr<TextModel> m_name_model;
      TextBox* m_name_input;
      AccountBox* m_account_box;
      Button* m_create_button;
      OverlayPanel* m_panel;
      boost::signals2::scoped_connection m_name_connection;

      void on_cancel();
      void on_create();
      void on_name_current(const QString& value);
  };
}

#endif
