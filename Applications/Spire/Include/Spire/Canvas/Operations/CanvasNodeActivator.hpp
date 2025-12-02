#ifndef SPIRE_CANVASNODEACTIVATOR_HPP
#define SPIRE_CANVASNODEACTIVATOR_HPP
#include <Beam/Pointers/Ref.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

class QWidget;

namespace Spire {

  //! Activates a CanvasNode.
  /*!
    \param node The CanvasNode to activate.
    \param userProfile The user's profile.
    \param parent The parent window.
  */
  void Activate(const CanvasNode& node, UserProfile& userProfile,
    QWidget* parent = nullptr);
}

#endif
