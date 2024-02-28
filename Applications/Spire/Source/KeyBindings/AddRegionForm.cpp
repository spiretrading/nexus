#include "Spire/KeyBindings/AddRegionForm.hpp"
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/OverlayPanel.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto make_action_button(const QString& name) {
    auto button = make_label_button(name);
    button->setAttribute(Qt::WA_NoMousePropagation);
    button->setFixedSize(scale(100, 26));
    return button;
  }
}

AddRegionForm::AddRegionForm(std::shared_ptr<ListModel<Region>> regions,
    QWidget& parent)
    : QWidget(&parent),
      m_regions(std::move(regions)) {
  auto heading = make_label(tr("Add Region"));
  heading->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  update_style(*heading, [] (auto& styles) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(12));
    styles.get(Any()).
      set(text_style(font, QColor(0x808080))).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(10)));
  });
  auto label = make_label(tr("Region"));
  m_region_drop_down_box = make_region_drop_down_box(m_regions);
  m_region_drop_down_box->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Fixed);
  find_focus_proxy(*m_region_drop_down_box)->installEventFilter(this);
  if(m_regions->get_size() > 0) {
    m_region_drop_down_box->get_current()->set(m_regions->get(0));
  }
  auto body = new QWidget();
  auto body_layout = make_vbox_layout(body);
  body_layout->setSpacing(scale_height(8));
  body_layout->addWidget(label);
  body_layout->addWidget(m_region_drop_down_box);
  auto field_set = new Box(body);
  update_style(*field_set, [] (auto& styles) {
    styles.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(PaddingTop(scale_height(10))).
      set(PaddingBottom(scale_height(18)));
  });
  auto cancel_button = make_action_button(tr("Cancel"));
  cancel_button->connect_click_signal(
    std::bind_front(&AddRegionForm::on_cancel, this));
  auto add_button = make_action_button(tr("Add"));
  add_button->connect_click_signal(
    std::bind_front(&AddRegionForm::on_add, this));
  auto actions_body = new QWidget();
  auto actions_layout = make_hbox_layout(actions_body);
  actions_layout->setSpacing(scale_height(8));
  actions_layout->addWidget(cancel_button);
  actions_layout->addWidget(add_button);
  auto actions_box = new Box(actions_body);
  update_style(*actions_box, [] (auto& styles) {
    styles.get(Any()).
      set(PaddingTop(scale_height(10))).
      set(PaddingBottom(scale_height(8))).
      set(horizontal_padding(scale_width(8)));
  });
  auto layout = make_vbox_layout(this);
  layout->addWidget(heading);
  layout->addWidget(field_set);
  layout->addWidget(actions_box);
  m_panel = new OverlayPanel(*this, parent);
  m_panel->setWindowModality(Qt::WindowModal);
  m_panel->set_positioning(OverlayPanel::Positioning::NONE);
  m_panel->set_closed_on_focus_out(false);
  m_panel->set_is_draggable(true);
  m_panel->installEventFilter(this);
  setFocusProxy(m_region_drop_down_box);
}

const std::shared_ptr<ListModel<Region>>& AddRegionForm::get_regions() const {
  return m_regions;
}

connection AddRegionForm::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool AddRegionForm::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_panel && event->type() == QEvent::Close) {
    hide();
  } else if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(key_event.key() == Qt::Key_Enter || key_event.key() == Qt::Key_Return) {
      on_add();
    }
  }
  return QWidget::eventFilter(watched, event);
}

bool AddRegionForm::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_panel->show();
    m_panel->activateWindow();
  } else if(event->type() == QEvent::HideToParent) {
    m_panel->hide();
  }
  return QWidget::event(event);
}

void AddRegionForm::on_cancel() {
  close();
}

void AddRegionForm::on_add() {
  m_submit_signal(m_region_drop_down_box->get_current()->get());
}
