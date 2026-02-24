#ifndef SPIRE_CANVAS_TYPE_REGISTRY_HPP
#define SPIRE_CANVAS_TYPE_REGISTRY_HPP
#include <Beam/Collections/View.hpp>
#include <boost/mp11/mpl_list.hpp>
#include <boost/optional/optional.hpp>
#include "Spire/Canvas/Records/Record.hpp"

namespace Spire {

  /** Lists all native types. */
  using NativeTypes = boost::mp11::mp_list<bool, Nexus::Quantity, double,
    boost::posix_time::ptime, boost::posix_time::time_duration, std::string,
    Nexus::CurrencyId, Nexus::Money, Nexus::OrderStatus, Nexus::OrderType,
    Nexus::Ticker, Nexus::Side, Nexus::TimeInForce, Nexus::Venue,
    Beam::Range, Beam::Sequence, std::shared_ptr<Nexus::Order>, Nexus::Asset,
    Record>;

  /** Lists all value types. */
  using ValueTypes = boost::mp11::mp_list<bool, Nexus::Quantity, double,
    boost::posix_time::ptime, boost::posix_time::time_duration, std::string,
    Nexus::CurrencyId, Nexus::Money, Nexus::OrderStatus, Nexus::OrderType,
    Nexus::Ticker, Nexus::Side, Nexus::TimeInForce, Nexus::Venue,
    Beam::Range, Beam::Sequence, Nexus::Asset, Record>;

  /** Lists types that can be compared. */
  using ComparableTypes = boost::mp11::mp_list<bool, Nexus::Quantity, double,
    boost::posix_time::ptime, boost::posix_time::time_duration, std::string,
    Nexus::Money, Beam::Sequence>;

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
