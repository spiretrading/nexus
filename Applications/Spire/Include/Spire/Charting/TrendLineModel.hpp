#ifndef SPIRE_TREND_LINE_MODEL_HPP
#define SPIRE_TREND_LINE_MODEL_HPP
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/TrendLine.hpp"
#include "Spire/Spire/Signal.hpp"

namespace Spire {

  //! Stores chart trend lines.
  class TrendLineModel {
    public:

      //! Signals that a trend line's property has changed.
      using UpdateSignal = Signal<void ()>;

      //! Constructs an empty TrendLineModel.
      TrendLineModel();

      //! Adds the provided trend line to the model and returns its id.
      /*!
        \param line The trend line to add to the model.
      */
      int add(const TrendLine& line);

      //! Returns the trend line with the provided id. Throws an exception
      //! if the provided id doesn't exist.
      /*!
        \param id The id of the trend line to return.
      */
      TrendLine get(int id);

      //! Returns the trend lines in the model.
      std::vector<TrendLine> get_lines() const;

      //! Returns a vector of ids representing the selected trend lines.
      std::vector<int> get_selected() const;

      //! Returns the id of the trend line that's closest to the provided point.
      //! Returns -1 if the model is empty.
      /*!
        \param point The point to compre the trend lines to.
      */
      int find_closest(const ChartPoint& point) const;

      //! Removes the trend line with the provided id from the model.
      //! Does nothing if the provided id doesn't exist.
      /*!
        \param id The id of the trend line to remove.
      */
      void remove(int id);

      //! Sets a trend lines status to selected.
      /*!
        \param id The id of the trend line to set as selected.
      */
      void set_selected(int id);

      //! Sets a trend lines status to unselected.
      /*!
        \param id the id of the trend line to set as unselected.
      */
      void unset_selected(int id);

      //! Toggles the selected status of the specified item.
      //! Does nothing if the specified id doesn't exist.
      /*!
        \param id The id of the trend line.
      */
      void toggle_selection(int id);

      //! Unsets the selected status of all selected trend lines.
      void clear_selected();

      //! Removes selected lines from the model.
      void remove_selected();

      //! Replaces the trend line at the specified id with the provided trend
      //! line. Does nothing if the specified id doesn't exist.
      /*!
        \param line The updated trend line.
        \param id The id of the trend line to replace.
      */
      void update(const TrendLine& line, int id);

      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const;

    private:
      enum class State {
        UNSELECTED,
        SELECTED
      };
      struct TrendLineEntry {
        TrendLine m_trend_line;
        int m_id;
        State m_state;
      };
      mutable UpdateSignal m_update_signal;
      int m_last_id;
      std::vector<TrendLineEntry> m_trend_lines;

      std::vector<TrendLineEntry>::iterator find_id(int id);
      bool is_selected(int id) const;
      void set_selected_status(int id, State state);
  };
}

#endif
