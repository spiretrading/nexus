#ifndef SPIRE_CANVASNODETRANSLATOR_HPP
#define SPIRE_CANVASNODETRANSLATOR_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslationContext.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

  //! Translates a CanvasNode.
  /*!
    \param context The context to translate the CanvasNode within.
    \param node The CanvasNode to translate.
    \return The translation of the <i>node</i>.
  */
  Translation Translate(CanvasNodeTranslationContext& context,
    const CanvasNode& node);
}

#endif
