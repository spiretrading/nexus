#ifndef SPIRE_DASHBOARDCELL_HPP
#define SPIRE_DASHBOARDCELL_HPP
#include <string>
#include <boost/circular_buffer.hpp>
#include <boost/noncopyable.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/variant/variant.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Dashboard/Dashboard.hpp"

namespace Spire {

  /*! \class DashboardCell
      \brief Represents a single cell within a dashboard.
   */
  class DashboardCell : private boost::noncopyable {
    public:

      //! The type used to represent a dashboard value.
      using Value = boost::variant<Nexus::Quantity, double, Nexus::Money,
        std::string, Nexus::Security>;

      //! Signals an update to this cell's value.
      /*!
        \param value The updated value.
      */
      using UpdateSignal = boost::signals2::signal<void (const Value& value)>;

      virtual ~DashboardCell() = default;

      //! Returns this cell's current value.
      virtual const boost::circular_buffer<Value>& GetValues() const = 0;

      //! Connects a slot to this cell's UpdateSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to this cell's UpdateSignal.
      */
      virtual boost::signals2::connection ConnectUpdateSignal(
        const UpdateSignal::slot_function_type& slot) const = 0;

    protected:

      //! Constructs a DashboardCell.
      DashboardCell() = default;
  };
}

#endif
