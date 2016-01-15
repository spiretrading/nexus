#ifndef SPIRE_TRANSLATIONPREPROCESSOR_HPP
#define SPIRE_TRANSLATIONPREPROCESSOR_HPP
#include <memory>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  //! Prepares a CanvasNode for translation.
  /*!
    \param node The CanvasNode to prepare.
    \return The CanvasNode to translate, or <code>null</code> if no
            preprocessing is needed.
   */
  std::unique_ptr<CanvasNode> PreprocessTranslation(const CanvasNode& node);
}

#endif
