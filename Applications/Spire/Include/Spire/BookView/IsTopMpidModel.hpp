#ifndef SPIRE_IS_TOP_MPID_MODEL_HPP
#define SPIRE_IS_TOP_MPID_MODEL_HPP
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/MpidBox.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

namespace Spire {

  /** Keeps track of whether an MPID at a given level is the top-most MPID. */
  class IsTopMpidModel : public ValueModel<bool> {
    public:

      /**
       * Constructs an IsTopMpidModel by checking if an MPID at a given level
       * is the top-most MPID based on a list top MPID levels.
       * @param top_mpid_levels The list of top levels for a every MPID.
       * @param mpid The specific MPID to track.
       * @param level The level to test for.
       */
      IsTopMpidModel(std::shared_ptr<TopMpidLevelListModel> top_mpid_levels,
        std::shared_ptr<MpidModel> mpid,
        std::shared_ptr<ValueModel<int>> level);

      const bool& get() const override;

      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<TopMpidLevelListModel> m_top_mpid_levels;
      std::shared_ptr<MpidModel> m_mpid;
      std::shared_ptr<ValueModel<int>> m_level;
      std::shared_ptr<ValueModel<TopMpidLevel>> m_top_mpid;
      LocalValueModel<bool> m_current;
      boost::signals2::scoped_connection m_mpid_connection;
      boost::signals2::scoped_connection m_level_connection;

      void on_mpid(const Mpid& mpid);
      void on_top_mpid(const TopMpidLevel& top);
      void on_level(int level);
  };
}

#endif
