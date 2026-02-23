#ifndef SPIRE_LASTPRICEDASHBOARDCELLBUILDER_HPP
#define SPIRE_LASTPRICEDASHBOARDCELLBUILDER_HPP
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCellBuilder.hpp"

namespace Spire {

  /*! \class LastPriceDashboardCellBuilder
      \brief Constructs a DashboardCell emitting a ticker's last price.
   */
  class LastPriceDashboardCellBuilder : public DashboardCellBuilder {
    public:

      //! Constructs a LastPriceDashboardCellBuilder.
      LastPriceDashboardCellBuilder() = default;

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
  void LastPriceDashboardCellBuilder::shuttle(S& shuttle,
      unsigned int version) {
    DashboardCellBuilder::shuttle(shuttle, version);
  }
}

#endif
