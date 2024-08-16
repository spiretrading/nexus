#ifndef SPIRE_VOLUMEDASHBOARDCELLBUILDER_HPP
#define SPIRE_VOLUMEDASHBOARDCELLBUILDER_HPP
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCellBuilder.hpp"

namespace Spire {

  /*! \class VolumeDashboardCellBuilder
      \brief Constructs a DashboardCell emitting a Security's volume.
   */
  class VolumeDashboardCellBuilder : public DashboardCellBuilder {
    public:

      //! Constructs a VolumeDashboardCellBuilder.
      VolumeDashboardCellBuilder() = default;

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
  void VolumeDashboardCellBuilder::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    DashboardCellBuilder::Shuttle(shuttle, version);
  }
}

#endif
