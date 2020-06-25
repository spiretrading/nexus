#ifndef SPIRE_RECENT_COLORS_HPP
#define SPIRE_RECENT_COLORS_HPP
#include <array>
#include <deque>
#include <QColor>
#include "Spire/Spire/Signal.hpp"

namespace Spire {

  //! Represents a data structure for storing recently selected colors in a
  //  FIFO manner.
  class RecentColors {
    public:

      //! Signals that the recent colors have changed.
      using ChangeSignal = Signal<void ()>;

      //! The maximum number of recent colors to store.
      static constexpr auto RECENT_COLOR_COUNT = 9;

      //! Returns the current recent colors.
      static RecentColors& get_instance();

      //! Inserts a color into the recently selected colors.
      /*!
        \param color The color to insert.
      */
      void add_color(const QColor& color);

      //! Returns the collection of recently selected colors.
      std::array<QColor, RECENT_COLOR_COUNT> get_colors() const;

      //! Connects a slot to the change signal.
      boost::signals2::connection connect_change_signal(
        const ChangeSignal::slot_type& slot) const;

      RecentColors(const RecentColors&) = delete;

      RecentColors& operator =(const RecentColors&) = delete;

    private:
      RecentColors();

      mutable ChangeSignal m_change_signal;
      std::deque<QColor> m_recent_colors;
  };
}

#endif
