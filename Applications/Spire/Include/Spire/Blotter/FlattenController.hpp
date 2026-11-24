#ifndef SPIRE_FLATTEN_CONTROLLER_HPP
#define SPIRE_FLATTEN_CONTROLLER_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include "Nexus/Definitions/Ticker.hpp"
#include "Spire/Blotter/BlotterCollection.hpp"

namespace Spire {
  class BlotterController;
  class UserProfile;

  /** Coordinates position flattening and outstanding flatten orders. */
  class FlattenController {
    public:

      /**
       * Constructs a FlattenController.
       * @param profile Supplies account snapshots and execution services.
       * @param collection Defines blotter ownership and links.
       */
      FlattenController(Beam::Ref<UserProfile> profile,
        std::shared_ptr<BlotterCollection> collection);

      ~FlattenController();

      /**
       * Flattens a blotter's projected position on the UI thread.
       * @param controller The managed blotter receiving the flatten task.
       * @param ticker The position to flatten.
       */
      void flatten(BlotterController& controller, const Nexus::Ticker& ticker);

    private:
      struct State;
      std::unique_ptr<State> m_state;

      FlattenController(const FlattenController&) = delete;
      FlattenController& operator =(const FlattenController&) = delete;
  };
}

#endif
