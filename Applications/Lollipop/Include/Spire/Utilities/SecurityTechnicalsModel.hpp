#ifndef SPIRE_SECURITY_TECHNICALS_MODEL_HPP
#define SPIRE_SECURITY_TECHNICALS_MODEL_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/signal.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Async/QtPromise.hpp"
#include "Spire/UI/UI.hpp"
#include "Spire/Utilities/Utilities.hpp"

namespace Spire {

  /** Models a Security's technical stats. */
  class SecurityTechnicalsModel {
    public:

      /**
       * Retrieves a SecurityTechnicalsModel.
       * @param userProfile The user's profile.
       * @param security The Security to model.
       */
      static std::shared_ptr<SecurityTechnicalsModel> GetModel(
        Beam::Ref<UserProfile> userProfile, const Nexus::Security& security);

      /**
       * Signals a change in the opening price.
       * @param open The updated opening price.
       */
      using OpenSignal = boost::signals2::signal<void (Nexus::Money open)>;

      /**
       * Signals a change in the closing price.
       * @param close The updated closing price.
       */
      using CloseSignal = boost::signals2::signal<void (Nexus::Money close)>;

      /**
       * Signals a change in the high price.
       * @param high The updated high price.
       */
      using HighSignal = boost::signals2::signal<void (Nexus::Money high)>;

      /**
       * Signals a change in the low price.
       * @param high The updated low price.
       */
      using LowSignal = boost::signals2::signal<void (Nexus::Money low)>;

      /**
       * Signals a change in the volume.
       * @param volume The updated volume.
       */
      using VolumeSignal =
        boost::signals2::signal<void (Nexus::Quantity volume)>;

      /**
       * Connects a slot to the OpenSignal.
       * @param slot The slot to connect.
       * @return A connection to the signal.
       */
      boost::signals2::connection
        ConnectOpenSignal(const OpenSignal::slot_type& slot) const;

      /**
       * Connects a slot to the CloseSignal.
       * @param slot The slot to connect.
       * @return A connection to the signal.
       */
      boost::signals2::connection
        ConnectCloseSignal(const CloseSignal::slot_type& slot) const;

      /**
       * Connects a slot to the HighSignal.
       * @param slot The slot to connect.
       * @return A connection to the signal.
       */
      boost::signals2::connection
        ConnectHighSignal(const HighSignal::slot_type& slot) const;

      /**
       * Connects a slot to the LowSignal.
       * @param slot The slot to connect.
       * @return A connection to the signal.
       */
      boost::signals2::connection
        ConnectLowSignal(const LowSignal::slot_type& slot) const;

      /**
       * Connects a slot to the VolumeSignal.
       * @param slot The slot to connect.
       * @return A connection to the signal.
       */
      boost::signals2::connection
        ConnectVolumeSignal(const VolumeSignal::slot_type& slot) const;

    private:
      UserProfile* m_userProfile;
      Nexus::Money m_open;
      Nexus::Money m_close;
      Nexus::Money m_high;
      Nexus::Money m_low;
      Nexus::Quantity m_volume;
      mutable OpenSignal m_openSignal;
      mutable CloseSignal m_closeSignal;
      mutable HighSignal m_highSignal;
      mutable LowSignal m_lowSignal;
      mutable VolumeSignal m_volumeSignal;
      std::shared_ptr<QtPromise<void>> m_loadPromise;
      EventHandler m_eventHandler;

      SecurityTechnicalsModel(
        Beam::Ref<UserProfile> userProfile, const Nexus::Security& security);
      void OnTimeAndSale(const Nexus::TimeAndSale& timeAndSale);
  };
}

#endif
