#ifndef SPIRE_SECURITYTECHNICALSMODEL_HPP
#define SPIRE_SECURITYTECHNICALSMODEL_HPP
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/TaskQueue.hpp>
#include <Beam/Threading/Sync.hpp>
#include <boost/signals2/signal.hpp>
#include <QObject>
#include <QTimer>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Utilities/Utilities.hpp"

namespace Spire {

  /*! \class SecurityTechnicalsModel
      \brief Models a Security's technical stats.
   */
  class SecurityTechnicalsModel : public QObject {
    public:

      //! Retrieves a SecurityTechnicalsModel.
      /*!
        \param userProfile The user's profile.
        \param security The Security to model.
      */
      static std::shared_ptr<SecurityTechnicalsModel> GetModel(
        Beam::Ref<UserProfile> userProfile,
        const Nexus::Security& security);

      //! Signals a change in the opening price.
      /*!
        \param open The updated opening price.
      */
      using OpenSignal = boost::signals2::signal<void (Nexus::Money open)>;

      //! Signals a change in the closing price.
      /*!
        \param close The updated closing price.
      */
      using CloseSignal = boost::signals2::signal<void (Nexus::Money close)>;

      //! Signals a change in the high price.
      /*!
        \param high The updated high price.
      */
      using HighSignal = boost::signals2::signal<void (Nexus::Money high)>;

      //! Signals a change in the low price.
      /*!
        \param high The updated low price.
      */
      using LowSignal = boost::signals2::signal<void (Nexus::Money low)>;

      //! Signals a change in the volume.
      /*!
        \param volume The updated volume.
      */
      using VolumeSignal = boost::signals2::signal<
        void (Nexus::Quantity volume)>;

      virtual ~SecurityTechnicalsModel();

      //! Connects a slot to the OpenSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectOpenSignal(
        const OpenSignal::slot_type& slot) const;

      //! Connects a slot to the CloseSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectCloseSignal(
        const CloseSignal::slot_type& slot) const;

      //! Connects a slot to the HighSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectHighSignal(
        const HighSignal::slot_type& slot) const;

      //! Connects a slot to the LowSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectLowSignal(
        const LowSignal::slot_type& slot) const;

      //! Connects a slot to the VolumeSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectVolumeSignal(
        const VolumeSignal::slot_type& slot) const;

    protected:
      SecurityTechnicalsModel(Beam::Ref<UserProfile> userProfile,
        const Nexus::Security& security);

    private:
      UserProfile* m_userProfile;
      std::shared_ptr<Beam::Threading::Sync<bool>> m_loadTechnicalsFlag;
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
      QTimer m_updateTimer;
      mutable Beam::TaskQueue m_slotHandler;

      void OnTimeAndSale(const Nexus::TimeAndSale& timeAndSale);
      void OnUpdateTimer();
  };
}

#endif
