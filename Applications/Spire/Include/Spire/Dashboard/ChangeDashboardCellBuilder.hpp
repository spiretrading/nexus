#ifndef SPIRE_CHANGEDASHBOARDCELLBUILDER_HPP
#define SPIRE_CHANGEDASHBOARDCELLBUILDER_HPP
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCellBuilder.hpp"

namespace Spire {

  /*! \class ChangeDashboardCellBuilder
      \brief Constructs a DashboardCell emitting the change in a Security's
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
      friend struct Beam::Serialization::DataShuttle;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void ChangeDashboardCellBuilder::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    DashboardCellBuilder::Shuttle(shuttle, version);
  }
}

#endif
