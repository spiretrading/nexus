#ifndef SPIRE_REGION_QUERY_MODEL_HPP
#define SPIRE_REGION_QUERY_MODEL_HPP
#include <string>
#include <unordered_set>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <boost/variant/variant.hpp>
#include <QAbstractTableModel>
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Spire/Async/QtPromise.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

namespace Spire {

  /** Queries available Regions. */
  class RegionQueryModel : public QAbstractTableModel {
    public:

      /** Enumerates the model's columns. */
      enum class Column {

        /** The region. */
        REGION,

        /** The name column. */
        NAME,

        /** The description column. */
        DESCRIPTION,
      };

      /** The number of columns available. */
      static const auto COLUMN_COUNT = 3;

      /**
       * Constructs a RegionQueryModel.
       * @param userProfile The user's profile.
       */
      explicit RegionQueryModel(Beam::Ref<UserProfile> userProfile);

      /**
       * Searches for Regions matching a prefix.
       * @param prefix The prefix to search for.
       */
      void Search(const std::string& prefix);

      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& parent) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      QVariant headerData(
        int section, Qt::Orientation orientation, int role) const override;

    private:
      using Item = boost::variant<
        Nexus::CountryCode, Nexus::Venue, Nexus::SecurityInfo, Nexus::Region>;
      UserProfile* m_userProfile;
      std::vector<Item> m_items;
      std::unordered_set<std::string> m_prefixes;
      QtPromise<void> m_queryPromise;

      void Add(std::vector<Item> items);
  };
}

#endif
