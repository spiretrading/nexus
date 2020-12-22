#ifndef SPIRE_BBOSIZEDASHBOARDCELLBUILDER_HPP
#define SPIRE_BBOSIZEDASHBOARDCELLBUILDER_HPP
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
      friend struct Beam::Serialization::DataShuttle;
      Nexus::Side m_side;

      BboSizeDashboardCellBuilder() = default;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void BboSizeDashboardCellBuilder::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    DashboardCellBuilder::Shuttle(shuttle, version);
    shuttle.Shuttle("side", m_side);
  }
}

#endif
