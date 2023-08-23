#include "Spire/Ui/ColorCodePanel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
}

ColorCodePanel::ColorCodePanel(QWidget* parent)
  : ColorCodePanel(std::make_shared<LocalValueModel<QColor>>(), parent) {}

ColorCodePanel::ColorCodePanel(std::shared_ptr<ValueModel<QColor>> current,
    QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_mode(Mode::HEX) {}

const std::shared_ptr<ValueModel<QColor>>& ColorCodePanel::get_current() const {
  return m_current;
}

ColorCodePanel::Mode ColorCodePanel::get_mode() const {
  return m_mode;
}

void ColorCodePanel::set_mode(Mode mode) {
  mode = m_mode;
}
