#include "Spire/Ui/LineInputForm.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QKeyEvent>
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

namespace {
  class NonEmptyTextModel : public TextModel {
    public:
      explicit NonEmptyTextModel(std::shared_ptr<TextModel> source)
        : m_source(std::move(source)),
          m_connection(m_source->connect_update_signal(m_update_signal)) {}

      QValidator::State get_state() const override {
        if(m_source->get().trimmed().isEmpty()) {
          return QValidator::State::Intermediate;
        }
        return m_source->get_state();
      }

      const QString& get() const override {
        return m_source->get();
      }

      QValidator::State test(const Type& value) const {
        if(value.trimmed().isEmpty()) {
          return QValidator::State::Intermediate;
        }
        return m_source->test(value);
      }

      QValidator::State set(const Type& value) {
        auto state = test(value);
        if(state == QValidator::State::Invalid) {
          return state;
        }
        m_source->set(value);
        return state;
      }

      connection connect_update_signal(
          const typename UpdateSignal::slot_type& slot) const {
        return m_update_signal.connect(slot);
      }

    private:
      mutable UpdateSignal m_update_signal;
      std::shared_ptr<TextModel> m_source;
      scoped_connection m_connection;

      void on_current(const QString& current) {
        m_update_signal(current);
      }
  };
}

LineInputForm::LineInputForm(QString heading, QWidget& parent)
  : LineInputForm(
      std::move(heading), std::make_shared<LocalTextModel>(), parent) {}

LineInputForm::LineInputForm(
    QString heading, std::shared_ptr<TextModel> current, QWidget& parent)
    : QWidget(&parent) {
  auto heading_label = make_label(std::move(heading));
  heading_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  update_style(*heading_label, [] (auto& styles) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(12));
    styles.get(Any()).
      set(text_style(font, QColor(0x808080))).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(10)));
  });
  auto label = make_label(tr("Name"));
  m_current = std::make_shared<NonEmptyTextModel>(std::move(current));
  m_input = new TextBox(m_current);
  m_input->installEventFilter(this);
  m_connection = m_current->connect_update_signal(
    std::bind_front(&LineInputForm::on_current, this));
  auto body = new QWidget();
  auto body_layout = make_vbox_layout(body);
  body_layout->setSpacing(scale_height(8));
  body_layout->addWidget(label);
  body_layout->addWidget(m_input);
  auto field_set = new Box(body);
  update_style(*field_set, [] (auto& styles) {
    styles.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(PaddingBottom(scale_height(8)));
  });
  auto cancel_button = make_label_button(tr("Cancel"));
  cancel_button->setFixedSize(scale(100, 26));
  cancel_button->connect_click_signal(
    std::bind_front(&LineInputForm::on_cancel, this));
  m_create_button = make_label_button(tr("Create"));
  m_create_button->setFixedSize(scale(100, 26));
  m_create_button->setEnabled(false);
  m_create_button->connect_click_signal(
    std::bind_front(&LineInputForm::on_create, this));
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
  layout->addWidget(heading_label);
  layout->addWidget(field_set);
  layout->addWidget(actions_box);
  m_panel = new OverlayPanel(*this, parent);
  m_panel->setWindowModality(Qt::WindowModal);
  m_panel->set_positioning(OverlayPanel::Positioning::NONE);
  m_panel->set_closed_on_focus_out(false);
  m_panel->set_is_draggable(true);
  m_panel->installEventFilter(this);
  setFocusProxy(m_input);
}

connection LineInputForm::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool LineInputForm::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_panel) {
    if(event->type() == QEvent::Close) {
      hide();
    }
  } else if(watched == m_input) {
    if(event->type() == QEvent::ChildAdded) {
      auto& child = *static_cast<QChildEvent&>(*event).child();
      if(child.isWidgetType()) {
        child.installEventFilter(this);
        m_input->removeEventFilter(this);
      }
    }
  } else if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(key_event.key() == Qt::Key_Enter || key_event.key() == Qt::Key_Return) {
      if(auto name = m_input->get_current()->get(); !name.trimmed().isEmpty()) {
        on_create();
        return true;
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

bool LineInputForm::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_current->set("");
    m_panel->show();
    m_panel->activateWindow();
  } else if(event->type() == QEvent::HideToParent) {
    auto current_blocker = shared_connection_block(m_connection);
    m_current->set("");
    m_panel->hide();
  }
  return QWidget::event(event);
}

void LineInputForm::on_cancel() {
  close();
}

void LineInputForm::on_create() {
  m_submit_signal(m_input->get_current()->get());
  close();
}

void LineInputForm::on_current(const QString& value) {
  m_create_button->setEnabled(!value.trimmed().isEmpty());
}
