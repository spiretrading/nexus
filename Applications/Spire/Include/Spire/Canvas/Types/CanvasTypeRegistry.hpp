#ifndef SPIRE_CANVAS_TYPE_REGISTRY_HPP
#define SPIRE_CANVAS_TYPE_REGISTRY_HPP
#include <Beam/Collections/View.hpp>
#include <boost/mpl/list.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Spire/Canvas/Records/Record.hpp"

namespace Spire {

  /** Lists all native types. */
  using NativeTypes = boost::mpl::list<bool, Nexus::Quantity, double,
    boost::posix_time::ptime, boost::posix_time::time_duration, std::string,
    Nexus::CurrencyId, Nexus::MarketCode, Nexus::Money, Nexus::OrderStatus,
    Nexus::OrderType, Nexus::Security, Nexus::Side, Nexus::TimeInForce,
    Beam::Queries::Range, Beam::Queries::Sequence,
    const Nexus::OrderExecutionService::Order*, Record>;

  /** Lists all value types. */
  using ValueTypes = boost::mpl::list<bool, Nexus::Quantity, double,
    boost::posix_time::ptime, boost::posix_time::time_duration, std::string,
    Nexus::CurrencyId, Nexus::MarketCode, Nexus::Money, Nexus::OrderStatus,
    Nexus::OrderType, Nexus::Security, Nexus::Side, Nexus::TimeInForce,
    Beam::Queries::Range, Beam::Queries::Sequence, Record>;

  /** Lists types that can be compared. */
  using ComparableTypes = boost::mpl::list<bool, Nexus::Quantity, double,
    boost::posix_time::ptime, boost::posix_time::time_duration, std::string,
    Nexus::Money, Beam::Queries::Sequence>;

  /** Stores the available CanvasTypes. */
  class CanvasTypeRegistry {
    public:

      /** Constructs a CanvasTypeRegistry. */
      CanvasTypeRegistry();

      /** Copies a CanvasTypeRegistry. */
      CanvasTypeRegistry(const CanvasTypeRegistry& registry) = default;

      /**
       * Finds a CanvasType with a specified name.
       * @param name The name of the CanvasType to find.
       * @return The CanvasType with the specified name.
       */
      boost::optional<const CanvasType&> Find(const std::string& name) const;

      /**
       * Finds a NativeType corresponding to a type_info instance.
       * @param type The type_info instance to lookup.
       * @return The NativeType mapping to the specified <i>type</i>.
       */
      boost::optional<const NativeType&> Find(const std::type_info& type) const;

      /**
       * Registers a CanvasType.
       * @param type The CanvasType to register.
       */
      void Register(const CanvasType& type);

      //! Returns the list of registered types.
      Beam::View<CanvasType> GetTypes() const;

      //! Returns the list of native types.
      Beam::View<NativeType> GetNativeTypes() const;

    private:
      std::vector<std::shared_ptr<CanvasType>> m_types;
      std::vector<std::shared_ptr<NativeType>> m_nativeTypes;
  };
}

#endif
