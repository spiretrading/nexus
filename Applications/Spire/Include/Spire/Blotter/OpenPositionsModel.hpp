#ifndef SPIRE_OPENPOSITIONSMODEL_HPP
#define SPIRE_OPENPOSITIONSMODEL_HPP
#include <optional>
#include <unordered_map>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/TaskQueue.hpp>
#include <QAbstractItemModel>
#include <QTimer>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class OpenPositionsModel
      \brief Models a blotter's open positions.
   */
  class OpenPositionsModel : public QAbstractTableModel {
    public:

      /*! \enum Columns
          \brief Enumerates the columns displayed in the open positions table.
       */
      enum Columns {

        //! The Security.
        SECURITY_COLUMN,

        //! The position's quantity.
        QUANTITY_COLUMN,

        //! The position's Side.
        SIDE_COLUMN,

        //! The average price of the position.
        AVERAGE_PRICE_COLUMN,

        //! The position's profit/loss.
        PROFIT_LOSS_COLUMN,

        //! The cost basis.
        COST_BASIS_COLUMN,

        //! The position's Currency.
        CURRENCY_COLUMN,
      };

      //! The number of columns available.
      static const unsigned int COLUMNS = 7;

      /*! \struct Entry
          \brief Stores an entry for a single open position.
       */
      struct Entry {

        //! The Entry's index.
        int m_index;

        //! The open position's Security.
        SpireBookkeeper::Key m_key;

        //! The Inventory representing the open position.
        SpireBookkeeper::Inventory m_inventory;

        //! The position's unrealized earnings.
        Nexus::Money m_unrealizedEarnings;

        //! Constructs an Entry.
        /*!
          \param index The Entry's index.
          \param key The open position's Security.
        */
        Entry(int index, const SpireBookkeeper::Key& key);
      };

      //! Constructs an OpenPositionsModel.
      OpenPositionsModel();

      virtual ~OpenPositionsModel();

      //! Sets the PortfolioController whose open positions are to be modeled.
      /*!
        \param portfolioController The PortfolioController whose open positions
               are to be modelled.
      */
      void SetPortfolioController(Beam::Ref<SpirePortfolioController>
        portfolioController);

      //! Returns all open positions.
      std::vector<Entry> GetOpenPositions() const;

      //! Returns the Entry for an open position.
      /*!
        \param security The Security whose open position Entry is to be
               retrieved.
        \return The Entry for the <i>security</i>'s open position iff it exists.
      */
      boost::optional<const Entry&> GetOpenPosition(
        const Nexus::Security& security) const;

      int rowCount(const QModelIndex& parent) const;

      int columnCount(const QModelIndex& parent) const;

      QVariant data(const QModelIndex& index, int role) const;

      QVariant headerData(int section, Qt::Orientation orientation,
        int role) const;

    private:
      SpirePortfolioController* m_portfolioController;
      QTimer m_updateTimer;
      std::vector<std::unique_ptr<Entry>> m_entries;
      std::unordered_map<Nexus::Security, Entry*> m_securityToEntry;
      std::optional<Beam::TaskQueue> m_slotHandler;

      void OnPortfolioUpdate(
        const SpirePortfolioController::UpdateEntry& update);
      void OnUpdateTimer();
  };
}

#endif
