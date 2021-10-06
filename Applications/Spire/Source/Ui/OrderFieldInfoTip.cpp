#include "Spire/Ui/OrderFieldInfoTip.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/InfoTip.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TextAreaBox.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto NAME_STYLE() {
    auto style = StyleSheet();
    auto font = QFont("Roboto");
    font.setWeight(60);
    font.setPixelSize(scale_width(12));
    style.get(ReadOnly()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(Font(font)).
      set(TextColor(QColor::fromRgb(0x808080))).
      set(horizontal_padding(0));
    return style;
  }

  auto PREREQUISITES_STYLE() {
    auto style = StyleSheet();
    auto font = QFont("Roboto");
    font.setPixelSize(scale_width(10));
    font.setItalic(true);
    style.get(ReadOnly()).
      set(border_color(QColor(Qt::transparent))).
      set(Font(font)).
      set(TextColor(QColor::fromRgb(0x4B23A0))).
      set(LineHeight(scale_height(1.13))).
      set(PaddingLeft(scale_width(20))).
      set(PaddingTop(scale_height(4)));
    return style;
  }

  auto make_value_widget(const OrderFieldInfoTip::Model::AllowedValue& value,
      QWidget* parent) {
    auto container = new QWidget(parent);
    container->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    auto layout = new QHBoxLayout(container);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    auto value_label =
      new TextBox(QString::fromStdString(value.m_value), container);
    value_label->set_read_only(true);
    value_label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    auto value_style = get_style(*value_label);
    value_style.get(ReadOnly()).
      set(border_size(0)).
      set(FontSize(scale_height(10))).
      set(TextAlign(Qt::AlignTop));
    set_style(*value_label, std::move(value_style));
    value_label->setFixedHeight(scale_height(48));
    layout->addWidget(value_label);
    auto description_label =
      new TextAreaBox(QString::fromStdString(value.m_description), container);
    description_label->set_read_only(true);
    description_label->setSizePolicy(
      QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    auto description_style = get_style(*description_label);
    description_style.get(ReadOnly()).
      set(FontSize(scale_height(10))).
      set(PaddingLeft(scale_width(8)));
    set_style(*description_label, std::move(description_style));
    layout->addWidget(description_label, 0, Qt::AlignTop);
    return container;
  }

  auto make_values_container(
      const std::vector<OrderFieldInfoTip::Model::AllowedValue>& values,
      QWidget* parent) {
    auto container = new QWidget(parent);
    auto layout = new QVBoxLayout(container);
    layout->setContentsMargins(
      scale_width(18), scale_height(18), scale_width(18), scale_height(18));
    layout->setSpacing(scale_height(8));
    for(const auto& value : values) {
      layout->addWidget(make_value_widget(value, container));
    }
    return container;
  }

  auto make_prerequisite_container(
      const QString& prerequisites, QWidget* parent) {
    auto container = new QWidget(parent);
    auto layout = new QVBoxLayout(container);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    auto header = make_label(QObject::tr("Prerequisites"), container);
    auto header_style = get_style(*header);
    header_style.get(ReadOnly()).
      set(FontSize(scale_height(10)));
    set_style(*header, std::move(header_style));
    layout->addWidget(header);
    auto prerequisites_label =
      new TextAreaBox(prerequisites, container);
    prerequisites_label->set_read_only(true);
    set_style(*prerequisites_label, PREREQUISITES_STYLE());
    layout->addWidget(prerequisites_label);
    auto box = new Box(container, parent);
    auto box_style = get_style(*box);
    box_style.get(Any()).
      set(BorderTopColor(QColor::fromRgb(0xE0E0E0))).
      set(BorderTopSize(scale_height(1))).
      set(padding(scale_width(18)));
    set_style(*box, std::move(box_style));
    return box;
  }
}

QString OrderFieldInfoTip::Model::get_prerequisites() const {
  auto text = QString();
  for(auto tag_index = std::size_t(0);
      tag_index < m_prerequisites.size(); ++tag_index) {
    auto tag = m_prerequisites[tag_index];
    text.append(QString::fromStdString(tag.m_name));
    if(!tag.m_values.empty()) {
      text.append(" = \"");
      for(auto i = std::size_t(0); i < tag.m_values.size(); ++i) {
        text.append(QString::fromStdString(tag.m_values[i].m_value));
        if(i < tag.m_values.size() - 1) {
          text.append(" | ");
        }
      }
      text.append("\"");
    }
    if(tag_index < m_prerequisites.size() - 1) {
      text.append(", ");
    }
  }
  return text;
}

OrderFieldInfoTip::OrderFieldInfoTip(Model model, QWidget* parent)
    : QWidget(parent) {
  setFixedWidth(280);
  auto container = new QWidget(this);
  container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto container_layout = new QVBoxLayout(container);
  container_layout->setContentsMargins({});
  container_layout->setSpacing(0);
  auto description_container = new QWidget(this);
  auto description_layout = new QVBoxLayout(description_container);
  description_layout->setContentsMargins(scale_width(18), scale_height(18),
    scale_width(18), 0);
  description_layout->setSpacing(0);
  auto name_label =
    make_label(QString::fromStdString(model.m_tag.m_name), this);
  name_label->set_read_only(true);
  set_style(*name_label, NAME_STYLE());
  description_layout->addWidget(name_label);
  auto description_label =
    new TextAreaBox(QString::fromStdString(model.m_tag.m_description), this);
  description_label->adjustSize();
  description_label->set_read_only(true);
  auto description_style = get_style(*description_label);
  description_style.get(ReadOnly()).
    set(PaddingTop(scale_height(6)));
  set_style(*description_label, std::move(description_style));
  description_layout->addWidget(description_label);
  container_layout->addWidget(description_container);
  if(!model.m_tag.m_values.empty()) {
    container_layout->addWidget(
      make_values_container(model.m_tag.m_values, this));
  }
  auto scroll_box = new ScrollBox(container, this);
  scroll_box->setMaximumHeight(scale_height(240));
  auto body_layout = new QVBoxLayout(this);
  body_layout->setSpacing(0);
  body_layout->setContentsMargins({});
  body_layout->addWidget(scroll_box);
  if(!model.m_prerequisites.empty()) {
    body_layout->addWidget(
      make_prerequisite_container(model.get_prerequisites(), this));
  }
  auto tip = new InfoTip(this, parent);
  tip->set_interactive(true);
}
