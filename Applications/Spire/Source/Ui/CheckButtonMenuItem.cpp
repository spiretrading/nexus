#include "Spire/Ui/CheckButtonMenuItem.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Spire;
using namespace Spire::Styles;

CheckButtonMenuItem::CheckButtonMenuItem(QString label, QWidget* parent)
  : CheckButtonMenuItem(std::move(label),
      std::make_shared<LocalBooleanModel>(), parent) {}

CheckButtonMenuItem::CheckButtonMenuItem(QString label,
    std::shared_ptr<BooleanModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)) {
  auto check_box = new CheckBox(m_current);
  check_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  check_box->setLayoutDirection(Qt::RightToLeft);
  check_box->set_label(std::move(label));
  update_style(*check_box, [] (auto& style) {
    style.get((Any() || Focus() || Hover()) > is_a<Box>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_color(QColor(Qt::transparent)));
  });
  enclose(*this, *check_box);
  proxy_style(*this, *check_box);
  setFocusProxy(check_box);
}

const std::shared_ptr<BooleanModel>& CheckButtonMenuItem::get_current() const {
  return m_current;
}
