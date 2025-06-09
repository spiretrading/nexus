#ifndef SPIRE_SEEK_BAR_HPP
#define SPIRE_SEEK_BAR_HPP
#include "Spire/Playback/Playback.hpp"
#include "Spire/Ui/DurationBox.hpp"
#include "Spire/Ui/HoverObserver.hpp"
#include "Spire/Ui/MouseMoveObserver.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Represents the timeline. */
  struct Timeline {

    /** The start time of the timeline. */
    boost::posix_time::ptime m_start;

    /** The duration of the timeline. */
    boost::posix_time::time_duration m_duration;
  };

  /** A ValueModel over a Timeline*/
  using TimelineModel = ValueModel<Timeline>;

  /** A LocalValueModel over a Timeline*/
  using LocalTimelineModel = LocalValueModel<Timeline>;

  /**
   * Represents a timeline along a track that the user can view and seek.
   */
  class SeekBar : public QWidget {
    public:

      /**
       * Constructs a SeekBar with the default position and modifiers.
       * @param timeline The timeline that the SeekBar represents.
       * @param parent The parent widget.
       */
      explicit SeekBar(std::shared_ptr<TimelineModel> timeline,
        QWidget* parent = nullptr);

      /**
       * Constructs a SeekBar.
       * @param timeline The timeline that the SeekBar represents.
       * @param current The current position as time offset from the start time.
       * @param modifiers The keyboard modifier increments.
       * @param parent The parent widget.
       */
      SeekBar(std::shared_ptr<TimelineModel> timeline,
        std::shared_ptr<DurationModel> current,
        QHash<Qt::KeyboardModifier, boost::posix_time::time_duration> modifiers,
        QWidget* parent = nullptr);

      /** Returns the timeline. */
      const std::shared_ptr<TimelineModel>& get_timeline() const;

      /** Returns the current position. */
      const std::shared_ptr<DurationModel>& get_current() const;

    private:
      struct SeekBarModel;
      Slider* m_slider;
      TextBox* m_tip;
      std::shared_ptr<SeekBarModel> m_model;
      boost::optional<HoverObserver> m_hover_observer;
      boost::optional<MouseMoveObserver> m_mouse_move_observer;

      void on_mouse_move(QWidget& target, QMouseEvent& event);
      void on_hover(HoverObserver::State state);
  };
}

#endif
