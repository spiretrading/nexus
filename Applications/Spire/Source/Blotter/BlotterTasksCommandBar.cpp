#include "Spire/Blotter/BlotterTasksCommandBar.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ToggleButton.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Spire;
using namespace Spire::Styles;

BlotterTasksCommandBar::BlotterTasksCommandBar(
    std::shared_ptr<BlotterModel> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_active(nullptr) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  auto content = new QWidget();
  auto layout = make_hbox_layout(content);
  m_active = make_icon_button(
    image_from_svg(":/Icons/blotter/tasks/active.svg", scale(26, 26)),
    tr("Set as active blotter"));
  m_active->setFixedSize(scale(26, 26));
  update_style(*m_active, [] (auto& style) {
    style.get((Checked() && !Hover() && !Press()) > Body()).
      set(BackgroundColor(QColor(0x7F5EEC)));
    style.get((Checked() && !Hover() && !Press()) > is_a<Icon>()).
      set(Fill(QColor(0xFFFFFF)));
    style.get((Checked() && Disabled()) > Body()).
      set(BackgroundColor(QColor(0xBAB3D9)));
    style.get((Checked() && Disabled()) > is_a<Icon>()).
      set(Fill(QColor(0xFFFFFF)));
  });
  m_active->connect_click_signal(
    std::bind_front(&BlotterTasksCommandBar::on_active, this));
  m_active_connection = m_model->is_active()->connect_update_signal(
    std::bind_front(&BlotterTasksCommandBar::on_active_update, this));
  on_active_update(m_model->is_active()->get());
  layout->addWidget(m_active);
  layout->addSpacing(scale_width(4));
  auto pin = make_icon_toggle_button(
    image_from_svg(":/Icons/blotter/tasks/pin.svg", scale(26, 26)),
    m_model->is_pinned(), tr("Pin blotter"));
  pin->setFixedSize(scale(26, 26));
  layout->addWidget(pin);
  layout->addSpacing(scale_width(8));
  auto separator = new Box();
  separator->setFixedSize(scale(1, 16));
  update_style(*separator, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xC8C8C8)));
  });
  layout->addWidget(separator, 0, Qt::AlignVCenter);
  layout->addSpacing(scale_width(8));
  auto run = make_icon_button(
    image_from_svg(":/Icons/blotter/tasks/run.svg", scale(26, 26)),
    tr("Run task"));
  run->setFixedSize(scale(26, 26));
  run->connect_click_signal(m_run_signal);
  update_style(*run, [] (auto& style) {
    style.get(Any() > is_a<Icon>()).set(Fill(QColor(0x1B9449)));
    style.get(Hover() > is_a<Icon>()).set(Fill(QColor(0x084D22)));
    style.get(Press() > is_a<Icon>()).set(Fill(QColor(0x0B7032)));
  });
  layout->addWidget(run);
  layout->addSpacing(scale_width(4));
  auto cancel = make_icon_button(
    image_from_svg(":/Icons/blotter/tasks/cancel.svg", scale(26, 26)),
    tr("Cancel tasks"));
  cancel->setAccessibleName(tr("Cancel tasks"));
  cancel->setFixedSize(scale(26, 26));
  cancel->connect_click_signal(m_cancel_signal);
  update_style(*cancel, [] (auto& style) {
    style.get(Any() > is_a<Icon>()).set(Fill(QColor(0xE01616)));
    style.get(Hover() > is_a<Icon>()).set(Fill(QColor(0x941B1B)));
    style.get(Press() > is_a<Icon>()).set(Fill(QColor(0xCF0404)));
  });
  layout->addWidget(cancel);
  layout->addStretch();
  auto body = new Box(content);
  update_style(*body, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(BorderBottomSize(scale_height(1))).
      set(BorderBottomColor(QColor(0xC8C8C8))).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  enclose(*this, *body);
}

boost::signals2::connection BlotterTasksCommandBar::connect_run_signal(
    const RunSignal::slot_type& slot) const {
  return m_run_signal.connect(slot);
}

boost::signals2::connection BlotterTasksCommandBar::connect_cancel_signal(
    const CancelSignal::slot_type& slot) const {
  return m_cancel_signal.connect(slot);
}

void BlotterTasksCommandBar::on_active() {
  m_model->is_active()->set(true);
}

void BlotterTasksCommandBar::on_active_update(bool is_active) {
  if(is_active) {
    match(*m_active, Checked());
  } else {
    unmatch(*m_active, Checked());
  }
}
