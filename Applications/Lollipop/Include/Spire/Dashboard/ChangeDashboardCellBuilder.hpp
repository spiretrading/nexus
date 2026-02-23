#ifndef SPIRE_CHANGEDASHBOARDCELLBUILDER_HPP
#define SPIRE_CHANGEDASHBOARDCELLBUILDER_HPP
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCellBuilder.hpp"

namespace Spire {

  /*! \class ChangeDashboardCellBuilder
      \brief Constructs a DashboardCell emitting the change in a ticker's
             price from it's previous day's closing price.
   */
  class ChangeDashboardCellBuilder : public DashboardCellBuilder {
    public:

      //! Constructs a ChangeDashboardCellBuilder.
      ChangeDashboardCellBuilder() = default;

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
  void ChangeDashboardCellBuilder::shuttle(S& shuttle, unsigned int version) {
    DashboardCellBuilder::shuttle(shuttle, version);
  }
}

#endif
