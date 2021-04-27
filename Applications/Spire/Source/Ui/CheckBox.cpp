#include "Spire/Ui/Checkbox.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/LocalValueModel.hpp"
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

CheckBox::CheckBox(QWidget* parent)
  : CheckBox(std::make_shared<LocalBooleanModel>(false), parent) {}

CheckBox::CheckBox(std::shared_ptr<BoolModel> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_is_read_only(false) {
  set_style(*this, DEFAULT_STYLE());
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  auto body = new QWidget(this);
  auto button = new Button(body, this);
  button->connect_clicked_signal([=] {
    m_model->set_current(!m_model->get_current());
  });
  layout->addWidget(button);
  m_body_layout = new QHBoxLayout(body);
  m_body_layout->setContentsMargins({});
  m_body_layout->setSpacing(0);
  m_check = create_check(m_model->get_current(), this);
  m_body_layout->addWidget(m_check);
  m_label = new TextBox(this);
  m_label->set_read_only(true);
  m_label->setDisabled(true);
  m_body_layout->addWidget(m_label);
  m_model->connect_current_signal([=] (auto is_checked) {
    on_checked(is_checked);
  });
}

void CheckBox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::LayoutDirectionChange) {
    // TODO: update text alignment
  }
}

const std::shared_ptr<BoolModel>& CheckBox::get_model() const {
  return m_model;
}

void CheckBox::set_label(const QString& label) {
  m_label->get_model()->set_current(label);
}

void CheckBox::set_read_only(bool is_read_only) {
  if(is_read_only != m_is_read_only) {
    m_is_read_only = is_read_only;
    if(m_is_read_only) {
      setAttribute(Qt::WA_TransparentForMouseEvents);
      setFocusPolicy(Qt::NoFocus);
    } else {
      setAttribute(Qt::WA_TransparentForMouseEvents, false);
      setFocusPolicy(Qt::StrongFocus);
    }
    // TODO: required?
    update();
  }
}

QSize CheckBox::sizeHint() const {
  return scale(80, 16);
}

connection CheckBox::connect_checked_signal(
    const CheckedSignal::slot_type& slot) const {
  return m_checked_signal.connect(slot);
}

void CheckBox::on_checked(bool is_checked) {
  delete_later(m_check);
  m_check = create_check(is_checked, this);
  if(m_body_layout->direction() == Qt::LeftToRight) {
    m_body_layout->insertWidget(0, m_check);
  } else {
    m_body_layout->addWidget(m_check);
  }
  m_checked_signal(is_checked);
}
