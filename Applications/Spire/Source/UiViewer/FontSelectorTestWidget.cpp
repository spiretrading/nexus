#include "Spire/UiViewer/FontSelectorTestWidget.hpp"
#include <QFontDatabase>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/UiViewer/UiViewer.hpp"

using namespace Spire;

namespace {
  auto CONTROL_SIZE() {
    static auto size = scale(100, 26);
    return size;
  }

  auto LABEL_SIZE() {
    static auto size = scale(260, 52);
    return size;
  }
}

FontSelectorTestWidget::FontSelectorTestWidget(QWidget* parent)
    : QWidget(parent),
      m_font_selector(nullptr) {
  auto container_widget = new QWidget(this);
  m_layout = new QGridLayout(container_widget);
  m_selected_label = new QLabel(tr("Selected Font"), this);
  m_selected_label->setMinimumSize(LABEL_SIZE());
  m_layout->addWidget(m_selected_label, 0, 2, 1, 1);
  m_preview_label = new QLabel(tr("Preview Font"), this);
  m_preview_label->setMinimumSize(LABEL_SIZE());
  m_layout->addWidget(m_preview_label, 1, 2, 1, 1);
  m_layout->addWidget(make_parameters_label(this), 1, 0, 1, 2);
  auto fonts = QFontDatabase().families();
  m_family_list = new StaticDropDownMenu(
    std::vector<QVariant>(fonts.begin(), fonts.end()), this);
  m_family_list->setFixedHeight(scale_height(26));
  m_layout->addWidget(m_family_list, 3, 0, 1, 2);
  m_size_spin_box = new IntegerSpinBox(std::make_shared<IntegerSpinBoxModel>(
    12, 6, 72, 1), this);
  m_size_spin_box->setFixedSize(CONTROL_SIZE());
  m_layout->addWidget(m_size_spin_box, 4, 0);
  m_bold_check_box = make_check_box(tr("Bold"), this);
  m_layout->addWidget(m_bold_check_box, 5, 0);
  m_italic_check_box = make_check_box(tr("Italic"), this);
  m_layout->addWidget(m_italic_check_box, 6, 0);
  m_underline_check_box = make_check_box(tr("Underline"), this);
  m_layout->addWidget(m_underline_check_box, 7, 0);
  m_set_button = make_flat_button(tr("Set Font"), this);
  m_set_button->setFixedSize(CONTROL_SIZE());
  m_set_button->connect_clicked_signal([=] { on_font_set(); });
  m_layout->addWidget(m_set_button, 8, 0);
  m_reset_button = make_flat_button(tr("Reset Widget"), this);
  m_reset_button->setFixedSize(CONTROL_SIZE());
  m_reset_button->connect_clicked_signal([=] { on_widget_reset(); });
  m_layout->addWidget(m_reset_button, 8, 1);
  on_widget_reset();
}

QFont FontSelectorTestWidget::get_font() const {
  auto font = QFont(m_family_list->get_current_item().value<QFont>());
  font.setBold(m_bold_check_box->isChecked());
  font.setItalic(m_italic_check_box->isChecked());
  font.setUnderline(m_underline_check_box->isChecked());
  font.setPointSize(static_cast<int>(m_size_spin_box->get_value()));
  return font;
}

void FontSelectorTestWidget::update_labels() {
  auto font = get_font();
  m_selected_label->setFont(font);
  m_preview_label->setFont(font);
}

void FontSelectorTestWidget::on_font_set() {
  m_font_selector->set_font(std::move(get_font()));
  update_labels();
}

void FontSelectorTestWidget::on_widget_reset() {
  delete_later(m_font_selector);
  auto font = get_font();
  m_font_selector = new FontSelectorWidget(font, this);
  m_font_selector->connect_font_preview_signal([=] (const auto& font) {
    m_preview_label->setFont(font);
  });
  m_font_selector->connect_font_selected_signal([=] (const auto& font) {
    m_selected_label->setFont(font);
  });
  m_layout->addWidget(m_font_selector, 0, 0, 1, 2);
  update_labels();
  setTabOrder(m_font_selector->findChild<IntegerSpinBox*>(), m_family_list);
  setTabOrder(m_family_list, m_size_spin_box);
  setTabOrder(m_size_spin_box, m_bold_check_box);
  setTabOrder(m_bold_check_box, m_italic_check_box);
  setTabOrder(m_italic_check_box, m_underline_check_box);
  setTabOrder(m_underline_check_box, m_set_button);
  setTabOrder(m_set_button, m_reset_button);
}
