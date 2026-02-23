#ifndef SPIRE_SCOPE_QUERY_MODEL_HPP
#define SPIRE_SCOPE_QUERY_MODEL_HPP
#include <string>
#include <unordered_set>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <boost/variant/variant.hpp>
#include <QAbstractTableModel>
#include "Nexus/Definitions/Scope.hpp"
#include "Nexus/Definitions/TickerInfo.hpp"
#include "Spire/Async/QtPromise.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

  /** Queries available Scopes. */
  class ScopeQueryModel : public QAbstractTableModel {
    public:

      /** Enumerates the model's columns. */
      enum class Column {

        /** The scope. */
        SCOPE,

        /** The name column. */
        NAME,

        /** The description column. */
        DESCRIPTION
      };

      /** The number of columns available. */
      static const auto COLUMN_COUNT = 3;

      /**
       * Constructs a ScopeQueryModel.
       * @param userProfile The user's profile.
       */
      explicit ScopeQueryModel(Beam::Ref<UserProfile> userProfile);

      /**
       * Searches for Scopes matching a prefix.
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
        Nexus::CountryCode, Nexus::Venue, Nexus::TickerInfo, Nexus::Scope>;
      UserProfile* m_userProfile;
      std::vector<Item> m_items;
      std::unordered_set<std::string> m_prefixes;
      QtPromise<void> m_queryPromise;

      void Add(std::vector<Item> items);
  };
}

#endif
