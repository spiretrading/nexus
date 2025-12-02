#ifndef SPIRE_HIGHDASHBOARDCELLBUILDER_HPP
#define SPIRE_HIGHDASHBOARDCELLBUILDER_HPP
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCellBuilder.hpp"

namespace Spire {

  /*! \class HighDashboardCellBuilder
      \brief Constructs a DashboardCell emitting a Security's daily high price.
   */
  class HighDashboardCellBuilder : public DashboardCellBuilder {
    public:

      //! Constructs a HighDashboardCellBuilder.
      HighDashboardCellBuilder() = default;

      virtual std::unique_ptr<DashboardCell> Make(
        const DashboardCell::Value& index,
        Beam::Ref<UserProfile> userProfile) const;

      virtual std::unique_ptr<DashboardCellBuilder> Clone() const;

    protected:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void HighDashboardCellBuilder::shuttle(S& shuttle,
      unsigned int version) {
    DashboardCellBuilder::shuttle(shuttle, version);
  }
}

#endif
