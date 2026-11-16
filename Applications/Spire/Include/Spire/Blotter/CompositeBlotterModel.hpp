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
       */
      explicit CompositeBlotterModel(std::shared_ptr<TextModel> name) noexcept;

      const std::shared_ptr<TextModel>& get_name() const override;

    private:
      std::shared_ptr<TextModel> m_name;
  };
}

#endif
