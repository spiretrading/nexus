#ifndef SPIRE_CANVASTYPEREGISTRY_HPP
#define SPIRE_CANVASTYPEREGISTRY_HPP
#include <Beam/Collections/View.hpp>
#include <boost/mpl/list.hpp>
#include <boost/optional/optional.hpp>
#include "Spire/Canvas/Records/Record.hpp"
#include "Spire/Canvas/TaskNodes/Task.hpp"

namespace Spire {

  //! Lists all native types.
  typedef boost::mpl::list<bool, Nexus::Quantity, double,
    boost::posix_time::ptime, boost::posix_time::time_duration, std::string,
    Task::State, Nexus::CurrencyId, Nexus::MarketCode, Nexus::Money,
    Nexus::OrderType, Nexus::OrderStatus, Nexus::Security, Nexus::Side,
    Nexus::TimeInForce, Record> NativeTypes;

  //! Lists all value types.
  typedef boost::mpl::list<bool, Nexus::Quantity, double,
    boost::posix_time::ptime, boost::posix_time::time_duration, std::string,
    Task::State, Nexus::CurrencyId, Nexus::MarketCode, Nexus::Money,
    Nexus::OrderType, Nexus::OrderStatus, Nexus::Security, Nexus::Side,
    Nexus::TimeInForce, Record> ValueTypes;

  //! Lists types that can be compared.
  typedef boost::mpl::list<bool, Nexus::Quantity, double,
    boost::posix_time::ptime, boost::posix_time::time_duration, std::string,
    Nexus::Money> ComparableTypes;

  /*! \class CanvasTypeRegistry
      \brief Stores the available CanvasTypes.
   */
  class CanvasTypeRegistry {
    public:

      //! Constructs a CanvasTypeRegistry.
      CanvasTypeRegistry();

      //! Copies a CanvasTypeRegistry.
      CanvasTypeRegistry(const CanvasTypeRegistry& registry) = default;

      //! Finds a CanvasType with a specified name.
      /*!
        \param name The name of the CanvasType to find.
        \return The CanvasType with the specified name.
      */
      boost::optional<const CanvasType&> Find(const std::string& name) const;

      //! Finds a NativeType corresponding to a type_info instance.
      /*!
        \param type The type_info instance to lookup.
        \return The NativeType mapping to the specified <i>type</i>.
      */
      boost::optional<const NativeType&> Find(const std::type_info& type) const;

      //! Registers a CanvasType.
      /*!
        \param type The CanvasType to register.
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
