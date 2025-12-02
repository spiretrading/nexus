#ifndef SPIRE_PROFITANDLOSSENTRYMODEL_HPP
#define SPIRE_PROFITANDLOSSENTRYMODEL_HPP
#include <boost/signals2/signal.hpp>
#include <QAbstractItemModel>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Blotter/Blotter.hpp"

namespace Spire {

  /*! \class ProfitAndLossEntryModel
      \brief Models a blotter's profit and loss for a single Currency.
   */
  class ProfitAndLossEntryModel : public QAbstractTableModel {
    public:

      //! Signals a change in the profit and loss.
      /*!
        \param profitAndLoss The new profit and loss.
      */
      using ProfitAndLossSignal = boost::signals2::signal<
        void (Nexus::Money profitAndLoss)>;

      //! Signals a change in the volume.
      /*!
        \param volume The total volume transacted.
      */
      using VolumeSignal = boost::signals2::signal<
        void (Nexus::Quantity volume)>;

      /*! \enum Columns
          \brief Enumerates the model's columns.
       */
      enum Columns {

        //! The Security.
        SECURITY_COLUMN,

        //! The profit and loss.
        PROFIT_AND_LOSS_COLUMN,

        //! The fees.
        FEES_COLUMN,

        //! The volume.
        VOLUME_COLUMN,
      };

      //! The number of columns.
      static const unsigned int COLUMN_COUNT = 4;

      //! Constructs a ProfitAndLossEntryModel.
      /*!
        \param currency The Currency being represented.
        \param showUnrealized Whether to factor unrealized profits.
      */
      ProfitAndLossEntryModel(const Nexus::CurrencyDatabase::Entry& currency,
        bool showUnrealized);

      virtual ~ProfitAndLossEntryModel();

      //! Returns this model's Currency.
      const Nexus::CurrencyDatabase::Entry& GetCurrency() const;

      //! Handles an update to this Currency's Portfolio.
      /*
        \param update The update to the Portfolio.
      */
      void OnPortfolioUpdate(const Nexus::PortfolioUpdateEntry& update);

      //! Connects a slot to the ProfitAndLossSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectProfitAndLossSignal(
        const ProfitAndLossSignal::slot_type& slot) const;

      //! Connects a slot to the VolumeSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectVolumeSignal(
        const VolumeSignal::slot_type& slot) const;

      int rowCount(const QModelIndex& parent) const;
      int columnCount(const QModelIndex& parent) const;
      QVariant data(const QModelIndex& index, int role) const;
      QVariant headerData(int section, Qt::Orientation orientation,
        int role) const;

    private:
      struct Entry {
        Nexus::Security m_security;
        Nexus::Money m_profitAndLoss;
        Nexus::Money m_fees;
        Nexus::Quantity m_volume;
        Nexus::Quantity m_previousQuantity;

        Entry(const Nexus::Security& security);
      };
      Nexus::CurrencyDatabase::Entry m_currency;
      bool m_showUnrealized;
      std::vector<std::shared_ptr<Entry>> m_entries;
      std::unordered_map<Nexus::Security, std::shared_ptr<Entry>>
        m_securityToEntry;
      Nexus::Quantity m_volume;
      mutable ProfitAndLossSignal m_profitAndLossSignal;
      mutable VolumeSignal m_volumeSignal;
  };
}

#endif
