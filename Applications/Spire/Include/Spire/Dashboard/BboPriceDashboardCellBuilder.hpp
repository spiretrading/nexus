#ifndef SPIRE_BBOPRICEDASHBOARDCELLBUILDER_HPP
#define SPIRE_BBOPRICEDASHBOARDCELLBUILDER_HPP
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCellBuilder.hpp"

namespace Spire {

  /*! \class BboPriceDashboardCellBuilder
      \brief Constructs a DashboardCell emitting a Security's BBO price.
   */
  class BboPriceDashboardCellBuilder : public DashboardCellBuilder {
    public:

      //! Constructs a BboPriceDashboardCellBuilder.
      /*!
        \param side The Side to emit.
      */
      BboPriceDashboardCellBuilder(Nexus::Side side);

      virtual std::unique_ptr<DashboardCell> Make(
        const DashboardCell::Value& index,
        Beam::Ref<UserProfile> userProfile) const;

      virtual std::unique_ptr<DashboardCellBuilder> Clone() const;

    protected:
      friend struct Beam::DataShuttle;
      Nexus::Side m_side;

      BboPriceDashboardCellBuilder() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void BboPriceDashboardCellBuilder::shuttle(S& shuttle, unsigned int version) {
    DashboardCellBuilder::shuttle(shuttle, version);
    shuttle.shuttle("side", m_side);
  }
}

#endif
