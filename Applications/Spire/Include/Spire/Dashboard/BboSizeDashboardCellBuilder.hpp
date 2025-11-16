#ifndef SPIRE_BBOSIZEDASHBOARDCELLBUILDER_HPP
#define SPIRE_BBOSIZEDASHBOARDCELLBUILDER_HPP
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCellBuilder.hpp"

namespace Spire {

  /*! \class BboSizeDashboardCellBuilder
      \brief Constructs a DashboardCell emitting a Security's BBO size.
   */
  class BboSizeDashboardCellBuilder : public DashboardCellBuilder {
    public:

      //! Constructs a BboSizeDashboardCellBuilder.
      /*!
        \param side The Side to emit.
      */
      BboSizeDashboardCellBuilder(Nexus::Side side);

      virtual std::unique_ptr<DashboardCell> Make(
        const DashboardCell::Value& index,
        Beam::Ref<UserProfile> userProfile) const;

      virtual std::unique_ptr<DashboardCellBuilder> Clone() const;

    protected:
      friend struct Beam::DataShuttle;
      Nexus::Side m_side;

      BboSizeDashboardCellBuilder() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void BboSizeDashboardCellBuilder::shuttle(S& shuttle, unsigned int version) {
    DashboardCellBuilder::shuttle(shuttle, version);
    shuttle.shuttle("side", m_side);
  }
}

#endif
