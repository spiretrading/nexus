#ifndef SPIRE_LOWDASHBOARDCELLBUILDER_HPP
#define SPIRE_LOWDASHBOARDCELLBUILDER_HPP
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCellBuilder.hpp"

namespace Spire {

  /*! \class LowDashboardCellBuilder
      \brief Builds a DashboardCell emitting a Security's daily low price.
   */
  class LowDashboardCellBuilder : public DashboardCellBuilder {
    public:

      //! Constructs a LowDashboardCellBuilder.
      LowDashboardCellBuilder() = default;

      virtual std::unique_ptr<DashboardCell> Build(
        const DashboardCell::Value& index,
        Beam::Ref<UserProfile> userProfile) const;

      virtual std::unique_ptr<DashboardCellBuilder> Clone() const;

    protected:
      friend struct Beam::Serialization::DataShuttle;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void LowDashboardCellBuilder::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    DashboardCellBuilder::Shuttle(shuttle, version);
  }
}

#endif
