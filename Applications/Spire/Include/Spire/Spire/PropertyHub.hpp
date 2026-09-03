#ifndef SPIRE_PROPERTY_HUB_HPP
#define SPIRE_PROPERTY_HUB_HPP
#include <any>
#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <boost/uuid/uuid.hpp>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /**
   * Stores a set of named properties shared among the components connected to
   * it.
   */
  class PropertyHub {
    public:

      /** The name of the property storing the ticker being displayed. */
      static const std::string TICKER_PROPERTY;

      /** Constructs a PropertyHub with a unique id. */
      PropertyHub();

      /**
       * Constructs a PropertyHub with a given id.
       * @param id The id uniquely identifying the hub.
       */
      explicit PropertyHub(boost::uuids::uuid id);

      /** Returns the id uniquely identifying this hub. */
      const boost::uuids::uuid& get_id() const;

      /**
       * Returns the model over a property, storing it with a default value if
       * no such property is stored.
       * @param name The name identifying the property.
       * @return The model over the property.
       * @throws <code>std::bad_any_cast</code> iff a property with the same
       *         name but a different type is stored.
       */
      template<typename T>
      std::shared_ptr<ValueModel<T>> get(const std::string& name);

      /**
       * Returns the model over a property without storing it.
       * @param name The name identifying the property.
       * @return The model over the property, or null if no such property is
       *         stored.
       */
      std::shared_ptr<AnyValueModel> find(const std::string& name) const;

      /**
       * Stores every property of another PropertyHub that is not already
       * stored by this PropertyHub.
       * @param hub The PropertyHub whose properties are to be stored.
       */
      void adopt(const PropertyHub& hub);

    private:
      boost::uuids::uuid m_id;
      std::unordered_map<std::string, std::shared_ptr<AnyValueModel>>
        m_properties;

      PropertyHub(const PropertyHub&) = delete;
      PropertyHub& operator =(const PropertyHub&) = delete;
  };

  template<typename T>
  std::shared_ptr<ValueModel<T>> PropertyHub::get(const std::string& name) {
    auto i = m_properties.find(name);
    if(i == m_properties.end()) {
      auto property = std::make_shared<LocalValueModel<T>>();
      m_properties.emplace(name, property);
      return property;
    }
    if(i->second->get().get_type() != typeid(T)) {
      throw std::bad_any_cast();
    }
    return std::static_pointer_cast<ValueModel<T>>(i->second);
  }
}

#endif
