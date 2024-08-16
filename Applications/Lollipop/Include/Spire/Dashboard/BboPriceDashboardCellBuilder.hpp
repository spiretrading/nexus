#ifndef SPIRE_BBOPRICEDASHBOARDCELLBUILDER_HPP
#define SPIRE_BBOPRICEDASHBOARDCELLBUILDER_HPP
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
      friend struct Beam::Serialization::DataShuttle;
      Nexus::Side m_side;

      BboPriceDashboardCellBuilder() = default;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void BboPriceDashboardCellBuilder::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    DashboardCellBuilder::Shuttle(shuttle, version);
    shuttle.Shuttle("side", m_side);
  }
}

#endif
