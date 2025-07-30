#include "Spire/Playback/TargetMenuItem.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

TargetMenuItem::TargetMenuItem(Target target, QWidget* parent)
  : TargetMenuItem(std::move(target), std::make_shared<LocalBooleanModel>(),
      parent) {}

TargetMenuItem::TargetMenuItem(Target target,
    std::shared_ptr<BooleanModel> current, QWidget* parent)
    : QWidget(parent),
      m_target(std::move(target)),
      m_current(std::move(current)),
BEAM_SUPPRESS_THIS_INITIALIZER()
      m_click_observer(*this) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
  setAttribute(Qt::WA_NoMousePropagation);
  auto body = new QWidget();
  auto layout = make_hbox_layout(body);
  if(m_target.m_color.isValid()) {
    auto marker = new Box();
    marker->setFixedSize(scale(10, 10));
    update_style(*marker, [&] (auto& style) {
      style.get(Any()).
        set(BackgroundColor(m_target.m_color)).
        set(border_radius(scale_width(5)));
    });
    layout->addWidget(marker, 0, Qt::AlignVCenter);
  } else {
    layout->addSpacing(scale_width(10));
  }
  layout->addSpacing(scale_width(6));
  auto check_box = new CheckBox(m_current);
  check_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  check_box->setLayoutDirection(Qt::RightToLeft);
  check_box->set_label(to_text(m_target));
  layout->addWidget(check_box, 0, Qt::AlignVCenter);
  auto box = new Box(body);
  enclose(*this, *box);
  proxy_style(*this, *box);
  setFocusProxy(check_box);
  update_style(*box, [] (auto& style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(5)));
  });
  m_click_observer.connect_click_signal(
    std::bind_front(&TargetMenuItem::on_click, this));
}

const TargetMenuItem::Target& TargetMenuItem::get_target() const {
  return m_target;
}

const std::shared_ptr<BooleanModel>& TargetMenuItem::get_current() const {
  return m_current;
}

void TargetMenuItem::on_click() {
  m_current->set(!m_current->get());
}

QString Spire::to_text(const TargetMenuItem::Target& target) {
  if(target.m_security != Security()) {
    if(target.m_count == 1) {
      return QObject::tr("%1 - %2").
        arg(::to_text(target.m_security)).
        arg(target.m_name);
    }
    return QObject::tr("%1 (%2)").
      arg(::to_text(target.m_security)).
      arg(target.m_count);
  } else if(target.m_count == 1) {
    return target.m_name;
  } else {
    return QObject::tr("Unassigned (%1)").arg(target.m_count);
  }
}
