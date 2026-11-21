#include "Spire/Blotter/BlotterPositionsCommandBar.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/Tooltip.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  Button* make_button(const QString& path, const QString& text,
      const KeySequenceValueModel& binding, ConnectionGroup& connections) {
    auto button = make_icon_button(image_from_svg(path, scale(26, 26)));
    button->setAccessibleName(text);
    button->setFixedSize(scale(26, 26));
    auto tooltip = new Tooltip("", button);
    auto update = [=] (const auto& sequence) {
      auto label = text;
      if(!sequence.isEmpty()) {
        label += QWidget::tr(" (%1)").arg(
          sequence.toString(QKeySequence::NativeText));
      }
      tooltip->set_label(label);
    };
    update(binding.get());
    connections.add(binding.connect_update_signal(update));
    return button;
  }
}

BlotterPositionsCommandBar::BlotterPositionsCommandBar(
    std::shared_ptr<ValueModel<FlattenState>> can_flatten,
    std::shared_ptr<PositionKeyBindingsModel> bindings, QWidget* parent)
    : QWidget(parent),
      m_can_flatten(std::move(can_flatten)),
      m_bindings(std::move(bindings)),
      m_flatten(nullptr),
      m_flatten_all(nullptr) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  auto content = new QWidget();
  auto layout = make_hbox_layout(content);
  m_flatten = make_button(
    ":/Icons/blotter/positions/flatten.svg", tr("Flatten position"),
    *m_bindings->get_binding(PositionKeyBindingsModel::Operation::FLATTEN),
    m_connections);
  m_flatten->connect_click_signal(m_flatten_signal);
  layout->addWidget(m_flatten);
  layout->addSpacing(scale_width(4));
  m_flatten_all = make_button(
    ":/Icons/blotter/positions/flatten_all.svg", tr("Flatten all positions"),
    *m_bindings->get_binding(PositionKeyBindingsModel::Operation::FLATTEN_ALL),
    m_connections);
  m_flatten_all->connect_click_signal(m_flatten_all_signal);
  layout->addWidget(m_flatten_all);
  layout->addStretch();
  on_update(m_can_flatten->get());
  m_connections.add(m_can_flatten->connect_update_signal(
    std::bind_front(&BlotterPositionsCommandBar::on_update, this)));
  auto body = new Box(content);
  update_style(*body, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(BorderBottomSize(scale_height(1))).
      set(BorderBottomColor(QColor(0xC8C8C8))).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  enclose(*this, *body);
}

connection BlotterPositionsCommandBar::connect_flatten_signal(
    const FlattenSignal::slot_type& slot) const {
  return m_flatten_signal.connect(slot);
}

connection BlotterPositionsCommandBar::connect_flatten_all_signal(
    const FlattenAllSignal::slot_type& slot) const {
  return m_flatten_all_signal.connect(slot);
}

void BlotterPositionsCommandBar::on_update(FlattenState state) {
  m_flatten->setEnabled(state == FlattenState::SELECTED_AND_ALL);
  m_flatten_all->setEnabled(state != FlattenState::NONE);
}
