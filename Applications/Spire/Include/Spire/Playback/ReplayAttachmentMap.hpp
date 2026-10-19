#ifndef SPIRE_REPLAY_ATTACHMENT_MAP_HPP
#define SPIRE_REPLAY_ATTACHMENT_MAP_HPP
#include <memory>
#include <unordered_map>
#include <vector>
#include "Spire/Playback/ReplaySession.hpp"
#include "Spire/Spire/PropertyHub.hpp"

namespace Spire {
  class PropertyHubMember;

  /**
   * Records which ReplaySession each PropertyHub is attached to. A PropertyHub
   * is attached to at most one ReplaySession, attaching it to another moves
   * it.
   */
  class ReplayAttachmentMap {
    public:

      /**
       * Signals a change to the ReplaySession a PropertyHub is attached to.
       * @param hub The PropertyHub whose attachment changed.
       * @param session The ReplaySession the hub is now attached to, null if
       *        it was detached.
       */
      using UpdateSignal = Signal<
        void (const std::shared_ptr<PropertyHub>& hub,
          const std::shared_ptr<ReplaySession>& session)>;

      /** Constructs an empty ReplayAttachmentMap. */
      ReplayAttachmentMap() = default;

      /**
       * Returns the ReplaySession a PropertyHub is attached to.
       * @param hub The PropertyHub to look up.
       * @return The ReplaySession the hub is attached to, null if there is
       *         none.
       */
      std::shared_ptr<ReplaySession> find(
        const std::shared_ptr<PropertyHub>& hub) const;

      /**
       * Returns the PropertyHubs attached to a ReplaySession, in no particular
       * order.
       * @param session The ReplaySession to look up.
       */
      std::vector<std::shared_ptr<PropertyHub>> get_hubs(
        const ReplaySession& session) const;

      /**
       * Attaches a PropertyHub to a ReplaySession, detaching it from the
       * ReplaySession it is currently attached to.
       * @param hub The PropertyHub to attach.
       * @param session The ReplaySession to attach the hub to, a null session
       *        detaches the hub.
       */
      void attach(std::shared_ptr<PropertyHub> hub,
        std::shared_ptr<ReplaySession> session);

      /**
       * Detaches a PropertyHub from the ReplaySession it is attached to.
       * @param hub The PropertyHub to detach.
       */
      void detach(const std::shared_ptr<PropertyHub>& hub);

      /** Connects a slot to the UpdateSignal. */
      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const;

    private:
      mutable UpdateSignal m_update_signal;
      std::unordered_map<std::shared_ptr<PropertyHub>,
        std::shared_ptr<ReplaySession>> m_sessions;

      ReplayAttachmentMap(const ReplayAttachmentMap&) = delete;
      ReplayAttachmentMap& operator =(const ReplayAttachmentMap&) = delete;
  };

  /**
   * Returns the list of PropertyHubs attached to a ReplaySession. Pushing a
   * PropertyHub attaches it to the session, detaching it from the session it
   * is currently attached to, and removing a PropertyHub detaches it. The list
   * updates when a PropertyHub is attached to or detached from the session
   * elsewhere.
   * @param attachment_map The ReplaySession every PropertyHub is attached
   *        to.
   * @param session The ReplaySession whose PropertyHubs are listed.
   */
  std::shared_ptr<PropertyHubListModel> make_attached_hub_list(
    std::shared_ptr<ReplayAttachmentMap> attachment_map,
    std::shared_ptr<ReplaySession> session);

  /**
   * Returns a model over the ReplaySession a member plays back: the session
   * its PropertyHub is attached to, otherwise null. The model updates when the
   * member joins another PropertyHub or the attachments change.
   * @param member The member whose session is modeled.
   * @param attachment_map The ReplaySession every PropertyHub is attached
   *        to.
   */
  std::shared_ptr<ReplaySessionModel> make_attached_session_model(
    const PropertyHubMember& member,
    std::shared_ptr<ReplayAttachmentMap> attachment_map);
}

#endif
