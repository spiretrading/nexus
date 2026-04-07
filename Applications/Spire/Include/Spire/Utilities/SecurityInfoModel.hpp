#ifndef SPIRE_SECURITY_INFO_MODEL_HPP
#define SPIRE_SECURITY_INFO_MODEL_HPP
#include <unordered_set>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <QAbstractTableModel>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Spire/Async/QtPromise.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Models the SecurityInfo's available on the MarketDataServer. */
  class SecurityInfoModel : public QAbstractTableModel {
    public:

      /** Enumerates the model's columns. */
      enum Columns {

        /** The Security column. */
        SECURITY_COLUMN,

        /** The name column. */
        NAME_COLUMN,

        /** The sector column. */
        SECTOR_COLUMN
      };

      /** The number of columns available. */
      static const auto COLUMN_COUNT = 3;

      /**
       * Constructs a SecurityInfoModel.
       * @param userProfile The user's profile.
       */
      explicit SecurityInfoModel(Beam::Ref<UserProfile> userProfile);

      /**
       * Searches for SecurityInfo objects matching a prefix.
       * @param prefix The prefix to search for.
       */
      void Search(const std::string& prefix);

      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& parent) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      QVariant headerData(
        int section, Qt::Orientation orientation, int role) const override;

    private:
      UserProfile* m_userProfile;
      std::vector<Nexus::SecurityInfo> m_securityInfoItems;
      std::unordered_set<std::string> m_prefixes;
      QtPromise<void> m_queryPromise;

      void AddSecurityInfoItems(
        std::vector<Nexus::SecurityInfo> securityInfoItems);
  };
}

#endif
