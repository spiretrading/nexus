#ifndef SPIRE_ACCOUNT_LIST_ITEM_HPP
#define SPIRE_ACCOUNT_LIST_ITEM_HPP
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <QImage>
#include <QString>
#include <QWidget>

namespace Spire {

  /** Represents an account list item. */
  class AccountListItem : public QWidget {
    public:

      /** Stores the display data for an account. */
      struct Account {

        /** The identicon image. */
        QImage m_identicon;

        /** The account's DirectoryEntry. */
        Beam::DirectoryEntry m_account;

        /** The full name associated with the account. */
        QString m_name;

        bool operator ==(const Account& other) const = default;
      };

      /**
       * Constructs an AccountListItem.
       * @param account The account to be displayed in the AccountListItem.
       * @param parent The parent widget.
       */
      explicit AccountListItem(Account account, QWidget* parent = nullptr);

      /** Returns the account. */
      const Account& get_account() const;

    private:
      Account m_account;
  };
}

#endif
