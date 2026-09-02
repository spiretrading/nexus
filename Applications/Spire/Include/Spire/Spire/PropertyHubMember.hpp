#ifndef SPIRE_PROPERTY_HUB_MEMBER_HPP
#define SPIRE_PROPERTY_HUB_MEMBER_HPP
#include <memory>
#include <QString>
#include <QWidget>
#include "Spire/Spire/PropertyHub.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {
  class UserProfile;

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
       * @param user_profile The user's profile.
       * @param component The component represented by this member.
       * @param name The name displayed for the component.
       * @param icon_path The path to the icon representing the component.
       * @param hub The PropertyHub to join.
       */
      PropertyHubMember(UserProfile& user_profile, QWidget& component,
        QString name, QString icon_path, std::shared_ptr<PropertyHub> hub);

      ~PropertyHubMember();

      /** Returns the component represented by this member. */
      QWidget& get_component() const;

      /** Returns the model over the name displayed for the component. */
      const std::shared_ptr<ValueModel<QString>>& get_name() const;

      /** Returns the path to the icon representing the component. */
      const QString& get_icon_path() const;

      /** Returns the model over the PropertyHub this component belongs to. */
      const std::shared_ptr<HubModel>& get_hub() const;

    private:
      UserProfile* m_user_profile;
      QWidget* m_component;
      std::shared_ptr<ValueModel<QString>> m_name;
      QString m_icon_path;
      std::shared_ptr<HubModel> m_hub;

      PropertyHubMember(const PropertyHubMember&) = delete;
      PropertyHubMember& operator =(const PropertyHubMember&) = delete;
  };
}

#endif
