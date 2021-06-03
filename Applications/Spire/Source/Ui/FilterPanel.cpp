#include "Spire/Ui/FilterPanel.hpp"
#include <QEvent>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto BUTTON_SIZE = QSize(102, 26);
  const auto HEADER_HEIGHT = 16;
  const auto MARGIN_SIZE = 8;
  const auto SPACE_BETWEEN_COMPONENT_BUTTON = 50;
  const auto SPACE_BETWEEN_HEADER_COMPONENT = 18;

  auto HEADER_STYLE() {
    auto style = StyleSheet();
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(12));
    style.get(Any()).
      set(border_size(0)).
      set(text_style(font, QColor::fromRgb(0x80, 0x80, 0x80)));
    return style;
  }
}

FilterPanel::FilterPanel(const QString& title, QWidget* component,
    QWidget* parent)
    : QWidget(parent),
      m_component(component) {
  auto layout = new QVBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins(scale_width(MARGIN_SIZE),
    scale_height(MARGIN_SIZE), scale_width(MARGIN_SIZE),
    scale_height(MARGIN_SIZE));
  auto header = new TextBox(title);
  header->setEnabled(false);
  header->set_read_only(true);
  set_style(*header, HEADER_STYLE());
  layout->addWidget(header);
  layout->addSpacing(scale_height(SPACE_BETWEEN_HEADER_COMPONENT));
  layout->addWidget(m_component);
  layout->addSpacing(scale_height(SPACE_BETWEEN_COMPONENT_BUTTON));
  auto reset_button = make_label_button(tr("Reset to Default"));
  reset_button->setFixedSize(scale(BUTTON_SIZE));
  layout->addWidget(reset_button, 0, Qt::AlignRight);
  reset_button->connect_clicked_signal([=] { m_reset_signal(); });
  auto max_width = std::max(component->width(),
    scale_width(BUTTON_SIZE.width()));
  setMinimumSize(max_width + scale_width(MARGIN_SIZE * 2),
    scale_height(MARGIN_SIZE * 2 + SPACE_BETWEEN_COMPONENT_BUTTON +
    SPACE_BETWEEN_HEADER_COMPONENT + BUTTON_SIZE.height() + HEADER_HEIGHT) +
    component->height());
  m_panel = new OverlayPanel(this, parent);
  m_panel->set_closed_on_blur(true);
}

const QWidget& FilterPanel::get_component() const {
  return *m_component;
}

QWidget& FilterPanel::get_component() {
  return *m_component;
}

connection FilterPanel::connect_reset_signal(
    const ResetSignal::slot_type& slot) const {
  return m_reset_signal.connect(slot);
}

bool FilterPanel::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_panel->show();
  }
  return QWidget::event(event);
}
