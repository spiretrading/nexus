#ifndef SPIRE_LISTDASHBOARDROWBUILDER_HPP
#define SPIRE_LISTDASHBOARDROWBUILDER_HPP
#include <vector>
#include <Beam/Serialization/ShuttleUniquePtr.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCellBuilder.hpp"
#include "Spire/Dashboard/DashboardRowBuilder.hpp"

namespace Spire {

  /*! \class ListDashboardRowBuilder
      \brief Constructs a DashboardRow from a list of DashboardCellBuilders.
   */
  class ListDashboardRowBuilder : public DashboardRowBuilder {
    public:

      //! Constructs a ListDashboardRowBuilder.
      /*!
        \param cellBuilders The list of DashboardCellBuilders.
      */
      ListDashboardRowBuilder(
        std::vector<std::unique_ptr<DashboardCellBuilder>> cellBuilders);

      virtual ~ListDashboardRowBuilder();

      virtual std::unique_ptr<DashboardRow> Make(
        const DashboardCell::Value& index,
        Beam::Ref<UserProfile> userProfile) const;

      virtual std::unique_ptr<DashboardRowBuilder> Clone() const;

    protected:
      friend struct Beam::Serialization::DataShuttle;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      std::vector<std::unique_ptr<DashboardCellBuilder>> m_cellBuilders;

      ListDashboardRowBuilder() = default;
  };

  template<typename Shuttler>
  void ListDashboardRowBuilder::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    DashboardRowBuilder::Shuttle(shuttle, version);
    shuttle.Shuttle("cell_builders", m_cellBuilders);
  }
}

#endif
