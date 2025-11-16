#ifndef SPIRE_DASHBOARDROWBUILDER_HPP
#define SPIRE_DASHBOARDROWBUILDER_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCell.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

  /*! \class DashboardRowBuilder
      \brief Constructs a DashboardRow.
   */
  class DashboardRowBuilder {
    public:
      virtual ~DashboardRowBuilder() = default;

      //! Return a DashboardRow with a specified index.
      /*!
        \param index The row's index.
        \param userProfile The user's profile.
        \return A DashboardRow with the specified <i>index</i>.
      */
      virtual std::unique_ptr<DashboardRow> Make(
        const DashboardCell::Value& index,
        Beam::Ref<UserProfile> userProfile) const = 0;

      //! Polymorphically clones this DashboardRowBuilder.
      virtual std::unique_ptr<DashboardRowBuilder> Clone() const = 0;

    protected:
      friend struct Beam::DataShuttle;

      //! Constructs a DashboardRowBuilder.
      DashboardRowBuilder() = default;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void DashboardRowBuilder::shuttle(S& shuttle, unsigned int version) {}
}

#endif
