#include "Spire/Ui/CheckButtonMenuItem.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Spire;
using namespace Spire::Styles;

CheckButtonMenuItem::CheckButtonMenuItem(QString label, QWidget* parent)
  : CheckButtonMenuItem(std::move(label),
      std::make_shared<LocalBooleanModel>(), parent) {}

CheckButtonMenuItem::CheckButtonMenuItem(QString label,
  std::shared_ptr<BooleanModel> current, QWidget* parent)
  : CheckButtonMenuItem(QImage(), std::move(label),
      std::make_shared<LocalBooleanModel>(), parent) {}

CheckButtonMenuItem::CheckButtonMenuItem(QImage icon, QString label,
    std::shared_ptr<BooleanModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
BEAM_SUPPRESS_THIS_INITIALIZER()
      m_click_observer(*this) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
  setAttribute(Qt::WA_NoMousePropagation);
  auto layout = make_hbox_layout(this);
  if(!icon.isNull()) {
    auto icon_widget = new Icon(std::move(icon));
    icon_widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    update_style(*icon_widget, [] (auto& style) {
      style.get(Any() || Hover()).set(Fill(QColor(Qt::black)));
    });
    layout->addWidget(icon_widget, 0, Qt::AlignVCenter);
    layout->addSpacing(scale_width(8));
  }
  auto check_box = new CheckBox(m_current);
  check_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  check_box->setLayoutDirection(Qt::RightToLeft);
  check_box->set_label(std::move(label));
  update_style(*check_box, [] (auto& style) {
    style.get((Any() || Focus() || Hover()) > is_a<Box>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_color(QColor(Qt::transparent)));
  });
  layout->addWidget(check_box, 0, Qt::AlignVCenter);
  proxy_style(*this, *check_box);
  setFocusProxy(check_box);
  m_click_observer.connect_click_signal(
    std::bind_front(&CheckButtonMenuItem::on_click, this));
}

const std::shared_ptr<BooleanModel>& CheckButtonMenuItem::get_current() const {
  return m_current;
}

void CheckButtonMenuItem::on_click() {
  m_current->set(!m_current->get());
}
