#include "Spire/Ui/Checkbox.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto& CHECK_ICON() {
    static auto icon = imageFromSvg(":/Icons/check.svg", scale(16, 16));
    return icon;
  }

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    return style;
  }

  auto create_check(bool is_checked, QWidget* parent) {
    auto check = [=] {
      if(is_checked) {
        return new Icon(CHECK_ICON(), parent);
      }
      return new Icon({}, parent);
    }();
    check->setFixedSize(scale(16, 16));
    return check;
  }
}

Checkbox::Checkbox(QWidget* parent)
  : Checkbox("", parent) {}

Checkbox::Checkbox(const QString& label, QWidget* parent)
    : QWidget(parent),
      m_is_checked(false),
      m_is_read_only(false) {
  set_style(*this, DEFAULT_STYLE());
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  auto body = new QWidget(this);
  auto button = new Button(body, this);
  button->connect_clicked_signal([=] { set_checked(!m_is_checked); });
  layout->addWidget(button);
  m_body_layout = new QHBoxLayout(body);
  m_body_layout->setContentsMargins({});
  m_body_layout->setSpacing(0);
  m_check = create_check(m_is_checked, this);
  m_body_layout->addWidget(m_check);
  m_label = new TextBox(label, this);
  m_label->set_read_only(true);
  m_label->setDisabled(true);
  m_body_layout->addWidget(m_label);
}

void Checkbox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::LayoutDirectionChange) {
    // TODO: update text alignment
  }
}

void Checkbox::set_checked(bool is_checked) {
  if(m_is_checked == is_checked) {
    return;
  }
  m_is_checked = is_checked;
  delete_later(m_check);
  m_check = create_check(m_is_checked, this);
  if(m_body_layout->direction() == Qt::LeftToRight) {
    m_body_layout->insertWidget(0, m_check);
  } else {
    m_body_layout->addWidget(m_check);
  }
  m_checked_signal(m_is_checked);
}

void Checkbox::set_label(const QString& label) {
  m_label->get_model()->set_current(label);
}

void Checkbox::set_read_only(bool is_read_only) {
  if(is_read_only != m_is_read_only) {
    m_is_read_only = is_read_only;
    if(m_is_read_only) {
      setAttribute(Qt::WA_TransparentForMouseEvents);
      setFocusPolicy(Qt::NoFocus);
    } else {
      setAttribute(Qt::WA_TransparentForMouseEvents, false);
      setFocusPolicy(Qt::StrongFocus);
    }
    update();
  }
}

QSize Checkbox::sizeHint() const {
  return scale(80, 16);
}

connection Checkbox::connect_checked_signal(
    const CheckedSignal::slot_type& slot) const {
  return m_checked_signal.connect(slot);
}
