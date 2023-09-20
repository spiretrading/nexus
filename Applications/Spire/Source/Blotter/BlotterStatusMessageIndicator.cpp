#include "Spire/Blotter/BlotterStatusMessageIndicator.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ToggleButton.hpp"

using namespace Spire;

BlotterStatusMessageIndicator::BlotterStatusMessageIndicator(QWidget* parent)
    : QWidget(parent) {
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  m_button = make_icon_toggle_button(
    imageFromSvg(":/Icons/blotter/bell.svg", scale(26, 26)));
  enclose(*this, *m_button);
}
