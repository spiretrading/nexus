#include "Spire/UiViewer/FilteredDropDownMenuTestWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/UiViewer/UiViewer.hpp"

using namespace Spire;

namespace {
  auto CONTROL_SIZE() {
    static auto size = scale(100, 26);
    return size;
  }

  auto get_variant_list(const QString& text) {
    auto strings = text.split(",", Qt::SkipEmptyParts);
    auto items = std::vector<QVariant>();
    items.reserve(strings.size());
    std::transform(strings.begin(), strings.end(),
      std::back_inserter(items),
      [] (auto string) {
        return QVariant::fromValue(string.replace(" ", ""));
      });
    return items;
  }
}

FilteredDropDownMenuTestWidget::FilteredDropDownMenuTestWidget(QWidget* parent)
    : QWidget(parent),
      m_menu(nullptr) {
  auto container = new QWidget(this);
  m_layout = new QGridLayout(container);
  m_status_label = new QLabel(this);
  m_layout->addWidget(m_status_label, 0, 1);
  m_layout->addWidget(make_parameters_label(this), 1, 0, 1, 2);
  m_item_input = new QPlainTextEdit(this);
  m_item_input->setPlainText("AA, AB, AC, BA, BB, BC, CA, CB, CC");
  m_item_input->setStyleSheet(QString(R"(
    QPlainTextEdit {
      background-color: #FFFFFF;
      border: %1px solid #C8C8C8 %2px solid #C8C8C8;
      color: #000000;
      font-family: Roboto;
      font-size: %3px;
    }

    QPlainTextEdit:focus {
      border: %1px solid #4B23A0 %2px solid #4B23A0;
    })").arg(scale_height(1)).arg(scale_width(1)).arg(scale_height(12)));
  m_item_input->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_layout->addWidget(m_item_input, 2, 0, 6, 2);
  m_set_button = make_flat_button(tr("Set Items"), this);
  m_set_button->setFixedSize(CONTROL_SIZE());
  m_set_button->connect_clicked_signal([=] { on_set_button(); });
  m_layout->addWidget(m_set_button, 8, 0);
  m_reset_button = make_flat_button(tr("Reset"), this);
  m_reset_button->setFixedSize(CONTROL_SIZE());
  m_reset_button->connect_clicked_signal([=] { on_reset_button(); });
  m_layout->addWidget(m_reset_button, 8, 1);
  on_reset_button();
}

void FilteredDropDownMenuTestWidget::on_set_button() {
  m_menu->set_items(get_variant_list(m_item_input->toPlainText()));
}

void FilteredDropDownMenuTestWidget::on_reset_button() {
  delete_later(m_menu);
  m_menu = new FilteredDropDownMenu(
    get_variant_list(m_item_input->toPlainText()), this);
  m_menu->setFixedSize(CONTROL_SIZE());
  connect(m_menu, &FilteredDropDownMenu::editingFinished, [=] {
    m_status_label->setText(m_menu->get_item().toString());
  });
  m_layout->addWidget(m_menu, 0, 0);
  setTabOrder(m_menu, m_item_input);
  setTabOrder(m_item_input, m_set_button);
  setTabOrder(m_set_button, m_reset_button);
}
