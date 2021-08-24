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
      set(BackgroundColor(QColor::fromRgb(255, 0, 0))).
      set(Font(font)).
      set(TextColor(QColor::fromRgb(0x808080)));
    return style;
  }

  auto PREREQUISITES_STYLE() {
    auto style = StyleSheet();
    auto font = QFont("Roboto");
    font.setPixelSize(scale_width(10));
    font.setItalic(true);
    style.get(ReadOnly()).
      set(BackgroundColor(QColor::fromRgb(255, 0, 0))).
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
    auto layout = new QHBoxLayout(container);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    auto value_label =
      new TextBox(QString::fromStdString(value.m_value), container);
    value_label->set_read_only(true);
    auto value_style = get_style(*value_label);
    value_style.get(ReadOnly()).
      set(BackgroundColor(QColor::fromRgb(255, 0, 0))).
      set(FontSize(scale_height(10))).
      set(TextAlign(Qt::AlignTop));
    set_style(*value_label, std::move(value_style));
    value_label->setFixedHeight(scale_height(48));
    layout->addWidget(value_label);
    auto description_label =
      new TextAreaBox(QString::fromStdString(value.m_description), container);
    description_label->set_read_only(true);
    auto description_style = get_style(*description_label);
    description_style.get(ReadOnly()).
      set(BackgroundColor(QColor::fromRgb(255, 0, 0))).
      set(FontSize(scale_height(10))).
      set(PaddingLeft(scale_width(8)));
    set_style(*description_label, std::move(description_style));
    layout->addWidget(description_label);
    return container;
  }

  auto make_values_container(
      const std::vector<OrderFieldInfoTip::Model::AllowedValue>& values,
      QWidget* parent) {
    auto container = new QWidget(parent);
    auto layout = new QVBoxLayout(container);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    for(const auto& value : values) {
      layout->addWidget(make_value_widget(value, container));
    }
    return container;
  }

  auto make_prerequisite_container(
      const std::vector<OrderFieldInfoTip::Model::Tag>& prerequisites,
      QWidget* parent) {
    auto container = new QWidget(parent);
    auto layout = new QVBoxLayout(container);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    auto header = new TextBox(QObject::tr("Prerequisites"), container);
    header->set_read_only(true);
    auto header_style = get_style(*header);
    header_style.get(ReadOnly()).
      set(BackgroundColor(QColor::fromRgb(255, 0, 0))).
      set(FontSize(scale_height(10)));
    set_style(*header, std::move(header_style));
    layout->addWidget(header);
    auto prerequisite_text = [&] {
      auto text = QString();
      for(auto tag_index = std::size_t(0);
          tag_index < prerequisites.size(); ++tag_index) {
        auto tag = prerequisites[tag_index];
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
        if(tag_index < prerequisites.size() - 1) {
          text.append(", ");
        }
      }
      return text;
    }();
    auto prerequisites_label = new TextAreaBox(prerequisite_text, container);
    prerequisites_label->set_read_only(true);
    set_style(*prerequisites_label, PREREQUISITES_STYLE());
    layout->addWidget(prerequisites_label);
    return container;
  }
}

OrderFieldInfoTip::OrderFieldInfoTip(Model model, QWidget* parent)
    : QWidget(parent) {
  auto container = new QWidget(this);
  auto container_layout = new QVBoxLayout(container);
  container_layout->setContentsMargins({});
  container_layout->setSpacing(0);
  auto name_label =
    new TextBox(QString::fromStdString(model.m_tag.m_name), this);
  name_label->set_read_only(true);
  set_style(*name_label, NAME_STYLE());
  container_layout->addWidget(name_label);
  auto description_label =
    new TextAreaBox(QString::fromStdString(model.m_tag.m_description), this);
  description_label->set_read_only(true);
  auto description_style = get_style(*description_label);
  description_style.get(ReadOnly()).
    set(PaddingTop(scale_height(6)));
  container_layout->addWidget(description_label);
  if(!model.m_tag.m_values.empty()) {
    container_layout->addWidget(
      make_values_container(model.m_tag.m_values, this));
  }
  auto scroll_box = new ScrollBox(container, this);
  auto body_layout = new QVBoxLayout(this);
  body_layout->setSpacing(0);
  body_layout->setContentsMargins({});
  body_layout->addWidget(scroll_box);
  if(!model.m_prerequisites.empty()) {
    auto prerequisites_container =
      new Box(make_prerequisite_container(model.m_prerequisites, this), this);
    body_layout->addWidget(prerequisites_container);
  }
  auto tip = new InfoTip(this, parent);
  tip->set_interactive(true);
}
