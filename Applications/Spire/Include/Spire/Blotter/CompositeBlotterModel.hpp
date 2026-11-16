#ifndef SPIRE_COMPOSITE_BLOTTER_MODEL_HPP
#define SPIRE_COMPOSITE_BLOTTER_MODEL_HPP
#include "Spire/Blotter/BlotterModel.hpp"

namespace Spire {

  /** Composes a blotter from supplied models. */
  class CompositeBlotterModel : public BlotterModel {
    public:

      /**
       * Constructs a CompositeBlotterModel.
       * @param name The blotter's name model.
       * @param is_active Whether the blotter is active.
       * @param is_pinned Whether the blotter is pinned for saving on exit.
       * @param tasks The blotter's order execution task entries.
       */
      CompositeBlotterModel(std::shared_ptr<TextModel> name,
        std::shared_ptr<BooleanModel> is_active,
        std::shared_ptr<BooleanModel> is_pinned,
        std::shared_ptr<TasksModel> tasks) noexcept;

      const std::shared_ptr<TextModel>& get_name() const override;
      const std::shared_ptr<BooleanModel>& is_active() const override;
      const std::shared_ptr<BooleanModel>& is_pinned() const override;
      const std::shared_ptr<TasksModel>& get_tasks() const override;

    private:
      std::shared_ptr<TextModel> m_name;
      std::shared_ptr<BooleanModel> m_is_active;
      std::shared_ptr<BooleanModel> m_is_pinned;
      std::shared_ptr<TasksModel> m_tasks;
  };
}

#endif
