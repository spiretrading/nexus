#ifndef SPIRE_DASHBOARDCELLBUILDER_HPP
#define SPIRE_DASHBOARDCELLBUILDER_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCell.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class DashboardCellBuilder
      \brief Constructs a DashboardCell.
   */
  class DashboardCellBuilder {
    public:
      virtual ~DashboardCellBuilder() = default;

      //! Returns a DashboardCell.
      /*!
        \param index The index of the row this cell belongs to.
        \param userProfile The user's profile.
        \return The DashboardCell represented by this builder.
      */
      virtual std::unique_ptr<DashboardCell> Make(
        const DashboardCell::Value& index,
        Beam::Ref<UserProfile> userProfile) const = 0;

      //! Polymorphically clones this DashboardRowBuilder.
      virtual std::unique_ptr<DashboardCellBuilder> Clone() const = 0;

    protected:
      friend struct Beam::Serialization::DataShuttle;

      //! Constructs a DashboardCellBuilder.
      DashboardCellBuilder() = default;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void DashboardCellBuilder::Shuttle(Shuttler& shuttle, unsigned int version) {}
}

#endif
