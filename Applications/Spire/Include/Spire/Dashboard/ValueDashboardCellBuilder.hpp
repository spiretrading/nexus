#ifndef SPIRE_VALUEDASHBOARDCELLBUILDER_HPP
#define SPIRE_VALUEDASHBOARDCELLBUILDER_HPP
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCellBuilder.hpp"

namespace Spire {

  /*! \class ValueDashboardCellBuilder
      \brief Constructs a ValueDashboardCell.
   */
  class ValueDashboardCellBuilder : public DashboardCellBuilder {
    public:

      //! Constructs a ValueDashboardCellBuilder.
      /*!
        \param value The value to assign to the DashboardCell.
      */
      ValueDashboardCellBuilder(const DashboardCell::Value& value);

      virtual std::unique_ptr<DashboardCell> Make(
        const DashboardCell::Value& index,
        Beam::Ref<UserProfile> userProfile) const;

      virtual std::unique_ptr<DashboardCellBuilder> Clone() const;

    protected:
      friend struct Beam::Serialization::DataShuttle;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      DashboardCell::Value m_value;

      ValueDashboardCellBuilder() = default;
  };

  template<typename Shuttler>
  void ValueDashboardCellBuilder::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    DashboardCellBuilder::Shuttle(shuttle, version);
    shuttle.Shuttle("value", m_value);
  }
}

#endif
