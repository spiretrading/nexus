#include "Spire/Ui/Slider.hpp"
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
//#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto make_modifiers(const ScalarValueModel<int>& current) {
    auto modifiers = QHash<Qt::KeyboardModifier, int>();
    modifiers[Qt::NoModifier] = current.get_increment();
    return modifiers;
  }

  auto get_layout_direction(Qt::Orientation orientation) {
    if(orientation == Qt::Orientation::Vertical) {
      return QBoxLayout::LeftToRight;
    }
    return QBoxLayout::TopToBottom;
  }

  auto get_distance(Qt::Orientation orientation, double value, double minimum,
      double maximum) {
    if(orientation == Qt::Orientation::Vertical) {
      return maximum - value;
    }
    return value - minimum;
  }

  auto get_value(Qt::Orientation orientation, double distance, double minimum,
      double maximum) {
    if(orientation == Qt::Orientation::Vertical) {
      return maximum - distance;
    }
    return minimum + distance;
  }

  auto get_position(Qt::Orientation orientation, const QPoint& point) {
    if(orientation == Qt::Orientation::Vertical) {
      return point.y();
    }
    return point.x();
  }

  auto get_size(Qt::Orientation orientation, const QSize& size) {
    if(orientation == Qt::Orientation::Vertical) {
      return size.height();
    }
    return size.width();
  }
}

Slider::Slider(QWidget* parent)
  : Slider(std::make_shared<LocalScalarValueModel<int>>(), parent) {}

Slider::Slider(QHash<Qt::KeyboardModifier, int> modifiers, QWidget* parent)
  : Slider(std::make_shared<LocalScalarValueModel<int>>(),
      std::move(modifiers), parent) {}

Slider::Slider(std::shared_ptr<ScalarValueModel<int>> current, QWidget* parent)
  : Slider(current, make_modifiers(*current), parent) {}

Slider::Slider(std::shared_ptr<ScalarValueModel<int>> current,
  QHash<Qt::KeyboardModifier, int> modifiers, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_modifiers(std::move(modifiers)),
      m_orientation(Qt::Horizontal),
      m_submission(m_current->get()),
      m_step_size(0),
      m_focus_observer(*this),
      m_is_dragging(false),
      m_is_modified(false) {
  setFocusPolicy(Qt::StrongFocus);
  auto body = new QWidget();
  m_track_body = new QLabel();
  m_track_body->setScaledContents(true);
  m_track_body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_track = new Box(m_track_body);
  m_track->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  match(*m_track, Track());
  m_track_layout = new QBoxLayout(get_layout_direction(m_orientation), body);
  m_track_layout->setContentsMargins({});
  m_track_layout->addStretch(1);
  m_track_layout->addWidget(m_track);
  m_track_layout->addStretch(1);
  auto thumb_icon = new Icon(QImage());
  m_thumb = new Box(thumb_icon, body);
  match(*m_thumb, Thumb());
  auto box = new Box(body);
  enclose(*this, *box);
  proxy_style(*this, *box);
  m_focus_observer.connect_state_signal(
    std::bind_front(&Slider::on_focus, this));
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&Slider::on_current, this));
  m_track_style_connection = connect_style_signal(*m_track,
    std::bind_front(&Slider::on_track_style, this));
  m_thumb_style_connection = connect_style_signal(*m_thumb,
    std::bind_front(&Slider::on_thumb_style, this));
  m_thumb_icon_style_connection = connect_style_signal(*thumb_icon,
    std::bind_front(&Slider::on_thumb_icon_style, this));
  m_style_connection = connect_style_signal(*this,
    std::bind_front(&Slider::on_style, this));
}

const std::shared_ptr<ScalarValueModel<int>>& Slider::get_current() const {
  return m_current;
}

int Slider::get_step_size() const {
  return m_step_size;
}

void Slider::set_step_size(int step_size) {
  m_step_size = step_size;
}

connection Slider::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void Slider::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Up || event->key() == Qt::Key_Right) {
    set_current(m_current->get() +
      get_increment(static_cast<int>(event->modifiers())));
  } else if(event->key() == Qt::Key_Down || event->key() == Qt::Key_Left) {
    set_current(m_current->get() -
      get_increment(static_cast<int>(event->modifiers())));
  } else if(event->key() == Qt::Key_PageUp) {
    set_current(m_current->get() + get_increment(Qt::ShiftModifier));
  } else if(event->key() == Qt::Key_PageDown) {
    set_current(m_current->get() - get_increment(Qt::ShiftModifier));
  } else if(event->key() == Qt::Key_Home) {
    m_current->set(*m_current->get_minimum());
  } else if(event->key() == Qt::Key_End) {
    m_current->set(*m_current->get_maximum());
  } else if(event->key() == Qt::Key_Escape) {
    set_current(m_submission);
    m_is_modified = false;
  } else {
    event->ignore();
  }
}

void Slider::mouseMoveEvent(QMouseEvent* event) {
  if(m_is_dragging) {
    auto current = to_value(::get_position(m_orientation,
      m_track->mapFromGlobal(event->globalPos())));
    if(auto change = current - m_current->get(); change != 0) {
      if(m_step_size != 0) {
        set_current(
          std::round((m_current->get() + change) / m_step_size) * m_step_size);
      } else {
        set_current(current);
      }
    }
  }
  QWidget::mouseMoveEvent(event);
}

void Slider::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    if(m_thumb->rect().contains(m_thumb->mapFromGlobal(event->globalPos()))) {
      m_is_dragging = true;
    } else {
      set_current(to_value(::get_position(m_orientation,
        m_track->mapFromGlobal(event->globalPos()))));
    }
  }
  QWidget::mousePressEvent(event);
}

void Slider::mouseReleaseEvent(QMouseEvent* event) {
  m_is_dragging = false;
  QWidget::mouseReleaseEvent(event);
}

void Slider::resizeEvent(QResizeEvent* event) {
  on_current(m_current->get());
  QWidget::resizeEvent(event);
}

void Slider::showEvent(QShowEvent* event) {
  on_current(m_current->get());
  QWidget::showEvent(event);
}

void Slider::wheelEvent(QWheelEvent* event) {
  set_current(m_current->get() +
    event->angleDelta().y() / 120.0 * m_current->get_increment());
}

int Slider::get_increment(int modifier_flag) const {
  if(auto increment =
      m_modifiers.find(static_cast<Qt::KeyboardModifier>(modifier_flag));
      increment != m_modifiers.end()) {
    return increment.value();
  }
  return m_modifiers[Qt::NoModifier];
}

double Slider::get_range() const {
  return static_cast<double>(*m_current->get_maximum()) -
    *m_current->get_minimum();
}

double Slider::to_value(int position) const {
  auto distance =
    position * get_range() / get_size(m_orientation, m_track->size());
  return get_value(m_orientation, distance, *m_current->get_minimum(),
    *m_current->get_maximum());
}

int Slider::to_position(double value) const {
  auto distance = get_distance(m_orientation, value, *m_current->get_minimum(),
    *m_current->get_maximum());
  return distance / get_range() * get_size(m_orientation, m_track->size()) -
      get_size(m_orientation, m_thumb->size()) / 2;
}

void Slider::set_current(double value) {
  auto current = static_cast<int>(std::clamp(value,
    static_cast<double>(*m_current->get_minimum()),
    static_cast<double>(*m_current->get_maximum())));
  if(current != m_current->get()) {
    m_current->set(current);
  }
}

void Slider::on_focus(FocusObserver::State state) {
  if(state == FocusObserver::State::NONE) {
    if(m_is_modified) {
      m_is_modified = false;
      m_submission = m_current->get();
      m_submit_signal(m_submission);
    }
  }
}

void Slider::on_current(int current) {
  m_is_modified = true;
  if(m_orientation == Qt::Horizontal) {
    m_thumb->move(to_position(current), (height() - m_thumb->height()) / 2);
  } else {
    m_thumb->move((width() - m_thumb->width()) / 2, to_position(current));
  }
}

void Slider::on_track_style() {
  auto& stylist = find_stylist(*m_track);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const IconImage& image) {
        stylist.evaluate(image, [=] (auto image) {
          m_track_body->setPixmap(QPixmap::fromImage(image));
          m_track_body->update();
        });
      });
  }
}

void Slider::on_thumb_style() {
  auto& stylist = find_stylist(*m_thumb);
  auto icon_image = std::make_shared<QImage>();
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const IconImage& image) {
        stylist.evaluate(image, [=] (auto image) {
          *icon_image = image;
        });
      });
  }
  if(icon_image) {
    auto current_blocker = shared_connection_block(m_thumb_style_connection);
    update_style(*m_thumb, [&] (auto& style) {
      style.get(Any() > is_a<Icon>()).set(IconImage(*icon_image));
    });
    m_thumb->setFixedSize(icon_image->size());
  }
}

void Slider::on_thumb_icon_style() {
  auto& stylist = find_stylist(*m_thumb->get_body());
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const IconImage& image) {
        stylist.evaluate(image, [=] (auto image) {
          m_thumb->setFixedSize(image.size());
        });
      });
  }
}

void Slider::on_style() {
  auto& stylist = find_stylist(*this);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (EnumProperty<Qt::Orientation> orientation) {
        stylist.evaluate(orientation, [=] (auto orientation) {
          if(m_orientation == orientation) {
            return;
          }
          m_orientation = orientation;
          m_track_body->setSizePolicy(m_track_body->sizePolicy().transposed());
          m_track_layout->setDirection(get_layout_direction(m_orientation));
          on_current(m_current->get());
        });
      });
  }
}
