#ifndef SPIRE_CLOSEDASHBOARDCELLBUILDER_HPP
#define SPIRE_CLOSEDASHBOARDCELLBUILDER_HPP
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCellBuilder.hpp"

namespace Spire {

  /*! \class CloseDashboardCellBuilder
      \brief Constructs a DashboardCell emitting a Security's closing price.
   */
  class CloseDashboardCellBuilder : public DashboardCellBuilder {
    public:

      //! Constructs a CloseDashboardCellBuilder.
      CloseDashboardCellBuilder() = default;

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
  void CloseDashboardCellBuilder::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    DashboardCellBuilder::Shuttle(shuttle, version);
  }
}

#endif
