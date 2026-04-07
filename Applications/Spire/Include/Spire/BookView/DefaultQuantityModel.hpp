#ifndef SPIRE_DEFAULT_QUANTITY_MODEL_HPP
#define SPIRE_DEFAULT_QUANTITY_MODEL_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QTimer>
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"

namespace Spire {

  /** Implements a model for a security's default quantity. */
  class DefaultQuantityModel :
      public ScalarValueModel<Nexus::Quantity>, private QObject {
    public:

      /**
       * Constructs a DefaultQuantityModel.
       * @param user_profile The profile to load the default quantity settings
       *        from.
       * @param security The security the default quantity is loaded for.
       * @param side The side of the position to keep track of.
       */
      DefaultQuantityModel(Beam::Ref<UserProfile> user_profile,
        Nexus::Security security, Nexus::Side side);

      const Type& get() const override;
      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;

    private:
      UserProfile* m_user_profile;
      Nexus::Security m_security;
      Nexus::Side m_side;
      LocalScalarValueModel<Nexus::Quantity> m_current;
      QTimer m_update_timer;

      void on_update();
  };
}

#endif
