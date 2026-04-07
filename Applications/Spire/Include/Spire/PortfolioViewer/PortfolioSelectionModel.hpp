#ifndef SPIRE_PORTFOLIOSELECTIONMODEL_HPP
#define SPIRE_PORTFOLIOSELECTIONMODEL_HPP
#include <array>
#include <unordered_set>
#include <Beam/Pointers/Ref.hpp>
#include <boost/variant/variant.hpp>
#include <QAbstractItemModel>
#include "Spire/PortfolioViewer/PortfolioViewer.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerModel.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerProperties.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class PortfolioSelectionModel
      \brief Stores the selection of Portfolio items to display.
   */
  class PortfolioSelectionModel : public QAbstractItemModel {
    public:

      //! The number of columns available.
      static const unsigned int COLUMN_COUNT = 1;

      /*! \enum SelectionTypes
          \brief Lists the types of selections available.
       */
      enum SelectionTypes {

        //! Select by group.
        GROUP_SELECTION = 0,

        //! Select by Currency.
        CURRENCY_SELECTION,

        //! Select by venue.
        VENUE_SELECTION,

        //! Select by Side.
        SIDE_SELECTION,
      };

      //! The number of selection types available.
      static const unsigned int SELECTION_TYPES_COUNT = 4;

      //! Constructs a PortfolioSelectionModel.
      /*!
        \param userProfile The user's profile.
        \param properties The properties used to model the selection.
        \param parent The parent object.
      */
      PortfolioSelectionModel(Beam::Ref<UserProfile> userProfile,
        const PortfolioViewerProperties& properties, QObject* parent = nullptr);

      virtual ~PortfolioSelectionModel();

      //! Modifies a specified PortfolioViewerProperties to reflect this
      //! model's selection parameters.
      /*!
        \param properties The properties to modify.
      */
      void UpdateProperties(
        Beam::Out<PortfolioViewerProperties> properties) const;

      //! Tests if a PortfolioViewerModel's Entry is part of this model's
      //! selection.
      /*!
        \param entry The Entry to test for acceptance.
        \return <code>true</code> iff the <i>entry</i> is accepted by this
                selection model.
      */
      bool TestSelectionAcceptsEntry(
        const PortfolioViewerModel::Entry& entry) const;

      virtual Qt::ItemFlags flags(const QModelIndex& index) const;

      virtual QModelIndex index(int row, int column,
        const QModelIndex& parent) const;

      virtual QModelIndex parent(const QModelIndex& index) const;

      virtual int rowCount(const QModelIndex& parent) const;

      virtual int columnCount(const QModelIndex& parent) const;

      virtual QVariant data(const QModelIndex& index, int role) const;

      virtual bool setData(const QModelIndex& index, const QVariant& value,
        int role = Qt::EditRole);

      virtual QVariant headerData(int section, Qt::Orientation orientation,
        int role) const;

    private:
      using SelectionVariant = boost::variant<
        Beam::DirectoryEntry, Nexus::CurrencyDatabase::Entry,
        Nexus::VenueDatabase::Entry, Nexus::Side>;
      UserProfile* m_userProfile;
      std::array<QModelIndex, SELECTION_TYPES_COUNT> m_roots;
      std::vector<Beam::DirectoryEntry> m_groups;
      std::unordered_set<Beam::DirectoryEntry> m_selectedGroups;
      std::unordered_set<Nexus::CurrencyId> m_selectedCurrencies;
      std::unordered_set<Nexus::Venue> m_selectedVenues;
      std::unordered_set<Nexus::Side> m_selectedSides;

      boost::optional<SelectionVariant> Find(const QModelIndex& index) const;
  };
}

#endif
