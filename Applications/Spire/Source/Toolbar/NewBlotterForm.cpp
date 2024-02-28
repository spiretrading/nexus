#include "Spire/Toolbar/NewBlotterForm.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QKeyEvent>
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

class NewBlotterForm::TextModel : public ::Spire::TextModel {
  public:
    QValidator::State get_state() const override {
      if(m_value.get().trimmed().isEmpty()) {
        return QValidator::State::Intermediate;
      }
      return m_value.get_state();
    }

    const QString& get() const override {
      return m_value.get();
    }

    QValidator::State test(const Type& value) const {
      if(value.trimmed().isEmpty()) {
        return QValidator::State::Intermediate;
      }
      return m_value.test(value);
    }

    QValidator::State set(const Type& value) {
      auto validation = m_value.set(value);
      if(value.trimmed().isEmpty()) {
        return QValidator::State::Intermediate;
      }
      return validation;
    }

    connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const {
      return m_value.connect_update_signal(slot);
    }

  private:
    LocalTextModel m_value;
};

NewBlotterForm::NewBlotterForm(
    std::shared_ptr<ListModel<BlotterModel*>> blotters, QWidget& parent)
    : QWidget(&parent),
      m_blotters(std::move(blotters)) {
  auto heading = make_label(tr("New Blotter"));
  heading->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  update_style(*heading, [] (auto& styles) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(12));
    styles.get(Any()).
      set(text_style(font, QColor(0x808080))).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(10)));
  });
  auto label = make_label(tr("Name"));
  m_current_name = std::make_shared<TextModel>();
  m_name = new TextBox(m_current_name);
  m_current_name_connection = m_current_name->connect_update_signal(
    std::bind_front(&NewBlotterForm::on_current_name, this));
  m_name->installEventFilter(this);
  auto body = new QWidget();
  auto body_layout = make_vbox_layout(body);
  body_layout->setSpacing(scale_height(8));
  body_layout->addWidget(label);
  body_layout->addWidget(m_name);
  auto field_set = new Box(body);
  update_style(*field_set, [] (auto& styles) {
    styles.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(PaddingBottom(scale_height(8)));
  });
  auto cancel_button = make_label_button(tr("Cancel"));
  cancel_button->setFixedSize(scale(100, 26));
  cancel_button->connect_click_signal(
    std::bind_front(&NewBlotterForm::on_cancel, this));
  m_create_button = make_label_button(tr("Create"));
  m_create_button->setFixedSize(scale(100, 26));
  m_create_button->setEnabled(false);
  m_create_button->connect_click_signal(
    std::bind_front(&NewBlotterForm::on_create, this));
  auto actions_body = new QWidget();
  auto actions_layout = make_hbox_layout(actions_body);
  actions_layout->setSpacing(scale_height(8));
  actions_layout->addWidget(cancel_button);
  actions_layout->addWidget(m_create_button);
  auto actions_box = new Box(actions_body);
  update_style(*actions_box, [] (auto& styles) {
    styles.get(Any()).
      set(PaddingTop(scale_height(10))).
      set(PaddingBottom(scale_height(8))).
      set(horizontal_padding(scale_width(8)));
  });
  auto layout = make_vbox_layout(this);
  layout->setSpacing(scale_height(10));
  layout->addWidget(heading);
  layout->addWidget(field_set);
  layout->addWidget(actions_box);
  m_panel = new OverlayPanel(*this, parent);
  m_panel->set_positioning(OverlayPanel::Positioning::NONE);
  m_panel->set_closed_on_focus_out(false);
  m_panel->set_is_draggable(true);
  m_panel->installEventFilter(this);
  setFocusProxy(m_name);
}

connection NewBlotterForm::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool NewBlotterForm::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_panel) {
    if(event->type() == QEvent::Close) {
      hide();
    }
  } else if(watched == m_name) {
    if(event->type() == QEvent::ChildAdded) {
      auto& child = *static_cast<QChildEvent&>(*event).child();
      if(child.isWidgetType()) {
        child.installEventFilter(this);
        m_name->removeEventFilter(this);
      }
    }
  } else if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(key_event.key() == Qt::Key_Enter || key_event.key() == Qt::Key_Return) {
      if(auto name = m_name->get_current()->get(); !name.trimmed().isEmpty()) {
        create_name(name);
        return true;
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

bool NewBlotterForm::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_current_name->set("");
    m_panel->show();
    m_panel->activateWindow();
  } else if(event->type() == QEvent::HideToParent) {
    auto current_blocker = shared_connection_block(m_current_name_connection);
    m_current_name->set("");
    m_panel->hide();
  }
  return QWidget::event(event);
}

void NewBlotterForm::create_name(const QString& name) {
  auto trimmed_name = name.trimmed();
  auto new_name = trimmed_name;
  auto count = 1;
  auto is_existing_name = true;
  while(is_existing_name) {
    is_existing_name = false;
    for(auto i = 0; i < m_blotters->get_size(); ++i) {
      if(m_blotters->get(i)->GetName() == new_name.toStdString()) {
        ++count;
        new_name = QString("%1 %2").arg(trimmed_name).arg(count);
        is_existing_name = true;
        break;
      }
    }
  }
  m_submit_signal(new_name);
  close();
}

void NewBlotterForm::on_cancel() {
  close();
}

void NewBlotterForm::on_create() {
  create_name(m_name->get_current()->get());
}

void NewBlotterForm::on_current_name(const QString& value) {
  if(value.trimmed().isEmpty()) {
    m_create_button->setEnabled(false);
  } else {
    m_create_button->setEnabled(true);
  }
}
