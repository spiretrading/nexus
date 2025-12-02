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
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);

    private:
      DashboardCell::Value m_value;

      ValueDashboardCellBuilder() = default;
  };

  template<Beam::IsShuttle S>
  void ValueDashboardCellBuilder::shuttle(S& shuttle,
      unsigned int version) {
    DashboardCellBuilder::shuttle(shuttle, version);
    shuttle.shuttle("value", m_value);
  }
}

#endif
