#include "Spire/UiViewer/FilteredDropDownMenuTestWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/FlatButton.hpp"

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
  m_layout->addWidget(m_item_input, 1, 0, 6, 2);
  auto set_button = make_flat_button(tr("Set Items"), this);
  set_button->setFixedSize(CONTROL_SIZE());
  set_button->connect_clicked_signal([=] { on_set_button(); });
  m_layout->addWidget(set_button, 7, 0);
  auto reset_button = make_flat_button(tr("Reset"), this);
  reset_button->setFixedSize(CONTROL_SIZE());
  reset_button->connect_clicked_signal([=] { on_reset_button(); });
  m_layout->addWidget(reset_button, 7, 1);
  m_reset_sort_order = [=] {
    setTabOrder(m_menu, m_item_input);
    setTabOrder(m_item_input, set_button);
    setTabOrder(set_button, reset_button);
  };
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
  m_reset_sort_order();
}
