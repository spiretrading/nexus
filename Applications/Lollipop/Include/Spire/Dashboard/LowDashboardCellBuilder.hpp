#ifndef SPIRE_LOWDASHBOARDCELLBUILDER_HPP
#define SPIRE_LOWDASHBOARDCELLBUILDER_HPP
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCellBuilder.hpp"

namespace Spire {

  /*! \class LowDashboardCellBuilder
      \brief Constructs a DashboardCell emitting a ticker's daily low price.
   */
  class LowDashboardCellBuilder : public DashboardCellBuilder {
    public:

      //! Constructs a LowDashboardCellBuilder.
      LowDashboardCellBuilder() = default;

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
  void LowDashboardCellBuilder::shuttle(S& shuttle,
      unsigned int version) {
    DashboardCellBuilder::shuttle(shuttle, version);
  }
}

#endif
