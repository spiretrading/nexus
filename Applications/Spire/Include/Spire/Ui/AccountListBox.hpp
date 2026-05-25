#ifndef SPIRE_ACCOUNT_LIST_BOX_HPP
#define SPIRE_ACCOUNT_LIST_BOX_HPP
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/AccountBox.hpp"

namespace Spire {
  template<typename> class TagComboBox;

  /** Represents a ListModel for a list of AccountListItem::Account values. */
  using AccountListModel = ListModel<AccountListItem::Account>;

  /** Displays a list of accounts over an open set of account values. */
  class AccountListBox : public QWidget {
    public:

      /**
       * Signals the submission of the list of accounts.
       * @param submission The list of submitted accounts.
       */
      using SubmitSignal =
        Signal<void (const std::shared_ptr<AccountListModel>& submission)>;

      /**
       * Constructs an AccountListBox using a default current.
       * @param accounts The set of accounts that can be queried.
       * @param parent The parent widget.
       */
      explicit AccountListBox(std::shared_ptr<AccountQueryModel> accounts,
        QWidget* parent = nullptr);

      /**
       * Constructs an AccountListBox.
       * @param accounts The set of accounts that can be queried.
       * @param current The current list of selected accounts.
       * @param parent The parent widget.
       */
      AccountListBox(std::shared_ptr<AccountQueryModel> accounts,
        std::shared_ptr<AccountListModel> current, QWidget* parent = nullptr);

      /** Returns the set of accounts that can be queried. */
      const std::shared_ptr<AccountQueryModel>& get_accounts() const;

      /** Returns the current list of accounts. */
      const std::shared_ptr<AccountListModel>& get_current() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& placeholder);

      /** Returns <code>true</code> iff this AccountListBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the AccountListBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    private:
      struct AccountToAccountIdQueryModel;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<AccountToAccountIdQueryModel> m_accounts;
      std::shared_ptr<AccountListModel> m_current;
      TagComboBox<QString>* m_tag_combo_box;
      boost::signals2::scoped_connection m_submit_connection;
  };
}

#endif
