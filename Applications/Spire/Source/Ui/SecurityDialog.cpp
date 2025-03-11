#include "Spire/Ui/SecurityDialog.hpp"
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

SecurityDialog::SecurityDialog(
  std::shared_ptr<SecurityInfoQueryModel> securities, QWidget* parent)
  : SecurityDialog(
      std::move(securities), std::make_shared<LocalSecurityModel>(), parent) {}

SecurityDialog::SecurityDialog(
    std::shared_ptr<SecurityInfoQueryModel> securities,
    std::shared_ptr<SecurityModel> current, QWidget* parent)
    : QWidget(parent) {
  auto header = make_label(tr("Security"));
  update_style(*header, [] (auto& style) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(12));
    style.get(ReadOnly() && Disabled()).
      set(text_style(font, QColor(0x808080))).
      set(PaddingBottom(scale_height(8)));
  });
  m_security_box = new SecurityBox(std::move(securities));
  update_style(*m_security_box, [] (auto& style) {
    style.get(Any() > (is_a<TextBox>() && !(+Any() << is_a<ListItem>()))).
      set(vertical_padding(scale_height(7)));
  });
  auto layout = make_vbox_layout(this);
  layout->addWidget(header);
  layout->addWidget(m_security_box);
  setFocusProxy(m_security_box);
  setFixedWidth(scale_width(180));
  m_panel = new OverlayPanel(*this, *parent);
  update_style(*m_panel, [] (auto& style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  m_panel->set_positioning(OverlayPanel::Positioning::NONE);
  m_panel->installEventFilter(this);
  m_input_box = find_focus_proxy(*m_security_box);
  m_input_box->installEventFilter(this);
}

const std::shared_ptr<SecurityInfoQueryModel>&
    SecurityDialog::get_securities() const {
  return m_security_box->get_securities();
}

const std::shared_ptr<SecurityModel>& SecurityDialog::get_current() const {
  return m_security_box->get_current();
}

connection SecurityDialog::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_security_box->connect_submit_signal(slot);
}

bool SecurityDialog::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_panel && event->type() == QEvent::Close) {
    close();
  } else if(watched == m_input_box && event->type() == QEvent::KeyPress) {
    auto& key_event = static_cast<QKeyEvent&>(*event);
    if(key_event.key() == Qt::Key_Escape) {
      QApplication::sendEvent(m_panel, event);
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

bool SecurityDialog::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_panel->show();
    auto parent_size = m_panel->parentWidget()->size();
    auto x = (parent_size.width() - m_panel->width()) / 2;
    auto y = (parent_size.height() - m_panel->height()) / 2;
    m_panel->move(m_panel->parentWidget()->mapToGlobal({x, y}));
    m_panel->activateWindow();
  } else if(event->type() == QEvent::HideToParent) {
    m_security_box->get_current()->set(Security());
    setFocus();
    m_panel->close();
  }
  return QWidget::event(event);
}
