#ifndef SPIRE_COMPOSITE_BLOTTER_MODEL_HPP
#define SPIRE_COMPOSITE_BLOTTER_MODEL_HPP
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterModel.hpp"

namespace Spire {

  /**
   * Implements the BlotterModel by composing constituent models together.
   */
  class CompositeBlotterModel : public BlotterModel {
    public:

      /**
       * Constructs a CompositeBlotterModel.
       * @param name Represents the name of this blotter.
       * @param status Represents the blotter's status/summary.
       */
      CompositeBlotterModel(std::shared_ptr<TextModel> name,
        std::shared_ptr<BlotterStatusModel> status);

      std::shared_ptr<TextModel> get_name() override;

      std::shared_ptr<BlotterStatusModel> get_status() override;

    private:
      std::shared_ptr<TextModel> m_name;
      std::shared_ptr<BlotterStatusModel> m_status;
  };

  /** Makes a CompositeBlotterModel using local models as its constituents. */
  std::shared_ptr<CompositeBlotterModel> make_local_blotter_model();
}

#endif
