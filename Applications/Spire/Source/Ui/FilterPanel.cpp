#include "Spire/Ui/FilterPanel.hpp"
#include <QEvent>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto MARGIN_SIZE = 8;

  auto HEADER_STYLE() {
    auto style = StyleSheet();
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(12));
    style.get(Any()).
      set(border_size(0)).
      set(text_style(font, QColor(0x808080)));
    return style;
  }
}

FilterPanel::FilterPanel(QString title, QWidget* body, QWidget& parent)
    : QWidget(&parent),
      m_body(body) {
  auto layout = new QVBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins(
    scale_width(MARGIN_SIZE), scale_height(MARGIN_SIZE),
    scale_width(MARGIN_SIZE), scale_height(MARGIN_SIZE));
  auto header = new TextBox(std::move(title));
  header->set_read_only(true);
  header->setFocusPolicy(Qt::NoFocus);
  set_style(*header, HEADER_STYLE());
  layout->addWidget(header);
  layout->addSpacing(scale_height(18));
  layout->addWidget(m_body);
  layout->addSpacing(scale_height(50));
  auto reset_button = make_label_button(tr("Reset to Default"));
  reset_button->setFixedHeight(scale_height(26));
  layout->addWidget(reset_button, 0, Qt::AlignRight);
  reset_button->connect_clicked_signal([=] { m_reset_signal(); });
  m_panel = new OverlayPanel(*this, parent);
  m_panel->set_closed_on_focus_out(true);
}

const QWidget& FilterPanel::get_body() const {
  return m_panel->get_body();
}

QWidget& FilterPanel::get_body() {
  return m_panel->get_body();
}

connection FilterPanel::connect_reset_signal(
    const ResetSignal::slot_type& slot) const {
  return m_reset_signal.connect(slot);
}

bool FilterPanel::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_panel->show();
    m_body->setFocus();
  }
  return QWidget::event(event);
}
