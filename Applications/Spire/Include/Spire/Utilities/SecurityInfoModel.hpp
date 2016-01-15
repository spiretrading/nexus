#ifndef SPIRE_SECURITYINFOMODEL_HPP
#define SPIRE_SECURITYINFOMODEL_HPP
#include <unordered_set>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/TaskQueue.hpp>
#include <Beam/Threading/Sync.hpp>
#include <QAbstractTableModel>
#include <QTimer>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class SecurityInfoModel
      \brief Models the SecurityInfo's available on the MarketDataServer.
   */
  class SecurityInfoModel : public QAbstractTableModel {
    public:

      /*! \enum Columns
          \brief Enumerates the model's columns.
       */
      enum Columns {

        //! The Security column.
        SECURITY_COLUMN,

        //! The name column.
        NAME_COLUMN,

        //! The sector column.
        SECTOR_COLUMN,
      };

      //! The number of columns available.
      static const unsigned int COLUMN_COUNT = 3;

      //! Constructs a SecurityInfoModel.
      /*!
        \param userProfile The user's profile.
      */
      SecurityInfoModel(Beam::RefType<UserProfile> userProfile);

      virtual ~SecurityInfoModel();

      //! Searches for SecurityInfo objects matching a prefix.
      /*!
        \param prefix The prefix to search for.
      */
      void Search(const std::string& prefix);

      int rowCount(const QModelIndex& parent) const;

      int columnCount(const QModelIndex& parent) const;

      QVariant data(const QModelIndex& index, int role) const;

      QVariant headerData(int section, Qt::Orientation orientation,
        int role) const;

    private:
      UserProfile* m_userProfile;
      std::vector<Nexus::SecurityInfo> m_securityInfoItems;
      std::unordered_set<std::string> m_prefixes;
      std::shared_ptr<Beam::Threading::Sync<bool>> m_isAliveFlag;
      QTimer m_updateTimer;
      Beam::TaskQueue m_slotHandler;

      void AddSecurityInfoItems(
        std::vector<Nexus::SecurityInfo> securityInfoItems);
      void OnUpdateTimer();
  };
}

#endif
