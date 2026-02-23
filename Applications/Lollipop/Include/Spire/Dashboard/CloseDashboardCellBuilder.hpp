#ifndef SPIRE_CLOSEDASHBOARDCELLBUILDER_HPP
#define SPIRE_CLOSEDASHBOARDCELLBUILDER_HPP
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCellBuilder.hpp"

namespace Spire {

  /*! \class CloseDashboardCellBuilder
      \brief Constructs a DashboardCell emitting a ticker's closing price.
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
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void CloseDashboardCellBuilder::shuttle(S& shuttle,
      unsigned int version) {
    DashboardCellBuilder::shuttle(shuttle, version);
  }
}

#endif
