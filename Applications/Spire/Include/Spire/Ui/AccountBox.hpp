#ifndef SPIRE_ACCOUNT_BOX_HPP
#define SPIRE_ACCOUNT_BOX_HPP
#include <QWidget>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/QueryModel.hpp"
#include "Spire/Ui/AccountListItem.hpp"

namespace Spire {
  template<typename> class ComboBox;

  /** Represents a QueryModel over AccountListItem::Account. */
  using AccountQueryModel = QueryModel<AccountListItem::Account>;

  /** Represents a ValueModel over AccountListItem::Account. */
  using AccountModel = ValueModel<AccountListItem::Account>;

  /** Represents a LocalValueModel over AccountListItem::Account. */
  using LocalAccountModel = LocalValueModel<AccountListItem::Account>;

  /** Displays an account over an open set of account values. */
  class AccountBox : public QWidget {
    public:

      /** A ValueModel over an AccountListItem::Account. */
      using CurrentModel = AccountModel;

      /**
       * Signals that the value was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal =
        Signal<void (const AccountListItem::Account& submission)>;

      /**
       * Constructs an AccountBox using a default local model.
       * @param accounts The set of accounts that can be queried.
       * @param parent The parent widget.
       */
      explicit AccountBox(std::shared_ptr<AccountQueryModel> accounts,
        QWidget* parent = nullptr);

      /**
       * Constructs an AccountBox.
       * @param accounts The set of accounts that can be queried.
       * @param current The current account displayed.
       * @param parent The parent widget.
       */
      AccountBox(std::shared_ptr<AccountQueryModel> accounts,
        std::shared_ptr<CurrentModel> current, QWidget* parent = nullptr);

      /** Returns the set of accounts that can be queried. */
      const std::shared_ptr<AccountQueryModel>& get_accounts() const;

      /** Returns the current model. */
      const std::shared_ptr<CurrentModel>& get_current() const;

      /** Returns the last submission. */
      const AccountListItem::Account& get_submission() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& placeholder);

      /** Returns true iff this AccountBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the AccountBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    private:
      struct AccountToAccountIdQueryModel;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<AccountToAccountIdQueryModel> m_accounts;
      std::shared_ptr<CurrentModel> m_current;
      AccountListItem::Account m_submission;
      ComboBox<QString>* m_combo_box;
      boost::signals2::scoped_connection m_submit_connection;
  };
}

#endif
