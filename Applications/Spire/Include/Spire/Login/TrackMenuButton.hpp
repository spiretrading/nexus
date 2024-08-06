#ifndef SPIRE_TRACK_MENU_BUTTON_HPP
#define SPIRE_TRACK_MENU_BUTTON_HPP
#include <memory>
#include <vector>
#include <QLabel>
#include <QWidget>
#include "Spire/Login/Login.hpp"
#include "Spire/Login/Track.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays the Spire logo and allows the selection of Spire track to run. */
  class TrackMenuButton : public QWidget {
    public:

      /** Enumerates the state of the button. */
      enum class State {

        /** The button is interactive. */
        READY,

        /** The button is non-interactive and displaying the load animation. */
        LOADING
      };

      /**
       * Constructs a TrackMenuButton in the READY state.
       * @param tracks The list of tracks the user can select.
       * @param current The currently selected track, must be equal to one of
       *        elements of <i>tracks</i>.
       * @param parent The parent widget.
       */
      TrackMenuButton(std::vector<Track> tracks,
        std::shared_ptr<TrackModel> current, QWidget* parent = nullptr);

      /** Returns the currently selected track. */
      const std::shared_ptr<TrackModel>& get_current() const;

      /** Returns the state of this button. */
      State get_state() const;

      /** Sets the state of this button. */
      void set_state(State state);

    private:
      std::shared_ptr<TrackModel> m_current;
      State m_state;
      MenuButton* m_button;
      QLabel* m_spinner;
  };
}

#endif
