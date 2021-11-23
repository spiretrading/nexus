#include "Spire/Ui/DropDownBox.hpp"
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownList.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any() >> is_a<Icon>()).
      set(Fill(QColor(0x333333))).
      set(BackgroundColor(QColor(Qt::transparent)));
    style.get(Disabled() >> is_a<Icon>()).set(Fill(QColor(0xC8C8C8)));
    style.get(ReadOnly() >> is_a<Icon>()).set(Visibility::NONE);
    style.get(Any() >> (is_a<TextBox>() && !(+Any() << is_a<ListItem>()))).
      set(PaddingRight(scale_width(14)));
    style.get(PopUp() >> is_a<TextBox>() ||
      (+Any() >> is_a<Button>() && (Hover() || FocusIn())) >> is_a<TextBox>()).
      set(border_color(QColor(0x4B23A0)));
    style.get(ReadOnly() >> (is_a<TextBox>() && !(+Any() << is_a<ListItem>()))).
      set(horizontal_padding(0)).
      set(border_color(QColor(Qt::transparent))).
      set(BackgroundColor(QColor(Qt::transparent)));
    return style;
  }
}

DropDownBox::DropDownBox(std::shared_ptr<ListModel> list, QWidget* parent)
  : DropDownBox(std::move(list), ListView::default_view_builder, parent) {}

DropDownBox::DropDownBox(
  std::shared_ptr<ListModel> list, ViewBuilder view_builder, QWidget* parent)
  : DropDownBox(std::move(list),
      std::make_shared<LocalValueModel<optional<int>>>(),
      std::make_shared<LocalValueModel<optional<int>>>(),
      std::move(view_builder), parent) {}

DropDownBox::DropDownBox(std::shared_ptr<ListModel> list,
    std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection, ViewBuilder view_builder,
    QWidget* parent)
    : QWidget(parent) {
  m_list_view = new ListView(std::move(list), std::move(current),
    std::move(selection), std::move(view_builder));
  m_text_box = new TextBox();
  m_text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_text_box->setFocusPolicy(Qt::NoFocus);
  auto layers = new LayeredWidget();
  layers->add(m_text_box);
  auto icon_layer = new QWidget();
  icon_layer->setAttribute(Qt::WA_TransparentForMouseEvents);
  icon_layer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto icon_layer_layout = new QHBoxLayout(icon_layer);
  icon_layer_layout->setContentsMargins({});
  icon_layer_layout->setSpacing(0);
  icon_layer_layout->addStretch();
  auto drop_down_icon =
    new Icon(imageFromSvg(":/Icons/dropdown-arrow.svg", scale(6, 4)));
  drop_down_icon->setFixedSize(scale(6, 4));
  icon_layer_layout->addWidget(drop_down_icon);
  icon_layer_layout->addSpacing(scale_width(8));
  layers->add(icon_layer);
  m_button = new Button(new QWidget());
  m_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layers->add(m_button);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(layers);
  m_drop_down_list = new DropDownList(*m_list_view, *this);
  m_drop_down_list->installEventFilter(this);
  auto window = m_drop_down_list->window();
  window->setWindowFlags(Qt::Popup | (window->windowFlags() & ~Qt::Tool));
  window->installEventFilter(this);
  set_style(*this, DEFAULT_STYLE());
  setFocusProxy(m_button);
  on_current(get_current()->get_current());
  m_button->connect_clicked_signal([=] { on_click(); });
  m_current_connection = m_list_view->get_current()->connect_current_signal(
      [=] (const auto& current) { on_current(current); });
  m_submit_connection = m_list_view->connect_submit_signal(
    [=] (const auto& submission) { on_submit(submission); });
  m_button->installEventFilter(this);
}

const std::shared_ptr<ListModel>& DropDownBox::get_list() const {
  return m_list_view->get_list();
}

const std::shared_ptr<DropDownBox::CurrentModel>&
    DropDownBox::get_current() const {
  return m_list_view->get_current();
}

const std::shared_ptr<DropDownBox::SelectionModel>&
    DropDownBox::get_selection() const {
  return m_list_view->get_selection();
}

bool DropDownBox::is_read_only() const {
  return m_text_box->is_read_only();
}

void DropDownBox::set_read_only(bool is_read_only) {
  m_text_box->set_read_only(is_read_only);
  if(is_read_only) {
    match(*this, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
  }
}

connection DropDownBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool DropDownBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_button) {
    if(event->type() == QEvent::FocusOut) {
      if(!is_read_only() && !m_drop_down_list->isVisible()) {
        submit();
        m_list_view->get_selection()->set_current(m_submission);
      }
    }
  } else if(watched == m_drop_down_list) {
    if(event->type() == QEvent::KeyPress) {
      if(!is_read_only()) {
        auto key = static_cast<QKeyEvent*>(event)->key();
        if(key == Qt::Key_Escape) {
          revert_current();
        } else {
          auto is_next = [&] {
            if(key == Qt::Key_Tab) {
              return optional<bool>(true);
            } else if(key == Qt::Key_Backtab) {
              return optional<bool>(false);
            }
            return optional<bool>();
          }();
          if(is_next) {
            m_drop_down_list->hide();
            focusNextPrevChild(*is_next);
          }
        }
      }
    }
  } else if(watched == m_drop_down_list->window()) {
    if(event->type() == QEvent::Close) {
      auto& close_event = static_cast<QCloseEvent&>(*event);
      close_event.ignore();
      m_drop_down_list->hide();
    } else if(event->type() == QEvent::Show) {
      match(*this, PopUp());
    } else if(event->type() == QEvent::Hide) {
      unmatch(*this, PopUp());
    } else if(event->type() == QEvent::MouseButtonPress) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      if(rect().contains(mapFromGlobal(mouse_event.globalPos()))) {
        m_drop_down_list->window()->setAttribute(Qt::WA_NoMouseReplay);
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void DropDownBox::keyPressEvent(QKeyEvent* event) {
  if(is_read_only()) {
    QWidget::keyPressEvent(event);
    return;
  }
  switch(event->key()) {
    case Qt::Key_Escape:
      revert_current();
      break;
    default:
      QCoreApplication::sendEvent(m_list_view, event);
  }
  QWidget::keyPressEvent(event);
}

void DropDownBox::on_click() {
  if(is_read_only()) {
    return;
  }
  if(m_drop_down_list->isVisible()) {
    m_drop_down_list->hide();
  } else {
    m_drop_down_list->show();
    m_drop_down_list->setFocus();
  }
}

void DropDownBox::on_current(const optional<int>& current) {
  auto text = [&] {
    if(current) {
      return displayTextAny(m_list_view->get_list()->at(*current));
    }
    return QString();
  }();
  m_text_box->get_current()->set_current(text);
}

void DropDownBox::on_submit(const std::any& submission) {
  if(is_read_only()) {
    return;
  }
  m_drop_down_list->hide();
  submit();
}

void DropDownBox::revert_current() {
  if(m_submission != m_list_view->get_current()->get_current()) {
    m_list_view->get_current()->set_current(m_submission);
  }
}

void DropDownBox::submit() {
  m_submission = m_list_view->get_current()->get_current();
  if(m_submission) {
    m_submit_signal(m_list_view->get_list()->at(*m_submission));
  }
}
