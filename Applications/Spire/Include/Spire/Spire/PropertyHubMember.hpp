#ifndef SPIRE_PROPERTY_HUB_MEMBER_HPP
#define SPIRE_PROPERTY_HUB_MEMBER_HPP
#include <any>
#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>
#include <boost/signals2/connection.hpp>
#include <QString>
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/PropertyHub.hpp"
#include "Spire/Spire/ProxyValueModel.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /**
   * Represents a component that shares its properties with the other
   * components belonging to the same PropertyHub.
   */
  class PropertyHubMember {
    public:

      /** The type of model over the PropertyHub a component belongs to. */
      using HubModel = ValueModel<std::shared_ptr<PropertyHub>>;

      /**
       * Constructs a PropertyHubMember.
       * @param roster The list of members to register with.
       * @param component The component represented by this member.
       * @param icon_path The path to the icon representing the component.
       * @param hub The PropertyHub to join.
       */
      PropertyHubMember(std::shared_ptr<ListModel<PropertyHubMember*>> roster,
        QWidget& component, QString icon_path,
        std::shared_ptr<PropertyHub> hub);

      ~PropertyHubMember();

      /** Returns the component represented by this member. */
      QWidget& get_component() const;

      /** Returns the name displayed for the component. */
      QString get_name() const;

      /** Returns the path to the icon representing the component. */
      const QString& get_icon_path() const;

      /** Returns the model over the PropertyHub this component belongs to. */
      const std::shared_ptr<HubModel>& get_hub() const;

      /**
       * Returns the model over a property shared with the components belonging
       * to the same PropertyHub.
       * @param name The name identifying the property.
       * @return The model over the property.
       * @throws <code>std::bad_any_cast</code> iff a property with the same
       *         name but a different type is stored.
       */
      template<typename T>
      std::shared_ptr<ValueModel<T>> get_property(const std::string& name);

    private:
      std::shared_ptr<ListModel<PropertyHubMember*>> m_roster;
      QWidget* m_component;
      QString m_icon_path;
      std::shared_ptr<HubModel> m_hub;
      std::unordered_map<std::string, std::shared_ptr<AnyValueModel>>
        m_properties;
      std::vector<boost::signals2::scoped_connection> m_connections;

      PropertyHubMember(const PropertyHubMember&) = delete;
      PropertyHubMember& operator =(const PropertyHubMember&) = delete;
  };

  template<typename T>
  std::shared_ptr<ValueModel<T>> PropertyHubMember::get_property(
      const std::string& name) {
    auto i = m_properties.find(name);
    if(i != m_properties.end()) {
      if(i->second->get().get_type() != typeid(T)) {
        throw std::bad_any_cast();
      }
      return std::static_pointer_cast<ValueModel<T>>(i->second);
    }
    auto property = make_proxy_value_model(m_hub->get()->get<T>(name));
    m_properties.emplace(name, property);
    m_connections.push_back(m_hub->connect_update_signal([=] (const auto& hub) {
      auto is_new = !hub->find(name);
      auto source = hub->get<T>(name);
      if(is_new) {
        source->set(property->get());
      }
      if(source != property->get_source()) {
        property->set_source(source);
      }
    }));
    return property;
  }
}

#endif
