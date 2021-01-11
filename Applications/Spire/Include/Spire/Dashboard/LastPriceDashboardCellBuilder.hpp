#ifndef SPIRE_LASTPRICEDASHBOARDCELLBUILDER_HPP
#define SPIRE_LASTPRICEDASHBOARDCELLBUILDER_HPP
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCellBuilder.hpp"

namespace Spire {

  /*! \class LastPriceDashboardCellBuilder
      \brief Constructs a DashboardCell emitting a Security's last price.
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
      friend struct Beam::Serialization::DataShuttle;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void LastPriceDashboardCellBuilder::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    DashboardCellBuilder::Shuttle(shuttle, version);
  }
}

#endif
