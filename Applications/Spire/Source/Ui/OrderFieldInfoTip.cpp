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
    font.setWeight(65);
    font.setPixelSize(scale_width(12));
    style.get(ReadOnly()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(Font(font)).
      set(TextColor(QColor(0x808080))).
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
      set(TextColor(QColor(0x4B23A0))).
      set(LineHeight(scale_height(1.13))).
      set(PaddingLeft(scale_width(20))).
      set(PaddingTop(scale_height(4)));
    return style;
  }

  auto make_description_container(const OrderFieldInfoTip::Model& model) {
    auto name_label = make_label(QString::fromStdString(model.m_tag.m_name));
    name_label->set_read_only(true);
    set_style(*name_label, NAME_STYLE());
    auto description_label =
      new TextAreaBox(QString::fromStdString(model.m_tag.m_description));
    description_label->set_read_only(true);
    auto description_style = get_style(*description_label);
    description_style.get(ReadOnly()).
      set(PaddingTop(scale_height(6))).
      set(PaddingBottom(0));
    set_style(*description_label, std::move(description_style));
    auto description_container = new QWidget();
    description_container->setSizePolicy(
      QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto description_layout = new QVBoxLayout(description_container);
    description_layout->setContentsMargins(
      scale_width(18), scale_height(18), scale_width(18), scale_height(18));
    description_layout->setSpacing(0);
    description_layout->addWidget(name_label);
    description_layout->addWidget(description_label);
    return description_container;
  }

  auto make_value_row(const OrderFieldInfoTip::Model::AllowedValue& value) {
    auto id_label = new TextBox(QString::fromStdString(value.m_value));
    id_label->set_read_only(true);
    id_label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    auto id_style = get_style(*id_label);
    id_style.get(ReadOnly()).
      set(vertical_padding(0)).
      set(border_size(0)).
      set(FontSize(scale_height(10))).
      set(TextAlign(Qt::AlignTop));
    set_style(*id_label, std::move(id_style));
    auto description_label =
      new TextAreaBox(QString::fromStdString(value.m_description));
    description_label->set_read_only(true);
    description_label->setSizePolicy(
      QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto description_style = get_style(*description_label);
    description_style.get(ReadOnly()).
      set(vertical_padding(0)).
      set(FontSize(scale_height(10))).
      set(PaddingLeft(scale_width(8)));
    set_style(*description_label, std::move(description_style));
    return std::tuple(id_label, description_label);
  }

  auto make_values_container(
      const std::vector<OrderFieldInfoTip::Model::AllowedValue>& values) {
    auto container = new QWidget();
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto layout = new QGridLayout(container);
    layout->setContentsMargins(
      scale_width(18), 0, scale_width(18), scale_height(18));
    layout->setHorizontalSpacing(0);
    layout->setVerticalSpacing(scale_height(8));
    for(auto i = std::size_t(0); i != values.size(); ++i) {
      auto& value = values[i];
      auto [id, description] = make_value_row(value);
      layout->addWidget(id, i, 0, Qt::AlignTop);
      layout->addWidget(description, i, 1, Qt::AlignTop);
    }
    return container;
  }

  auto make_scroll_box_body(const OrderFieldInfoTip::Model& model) {
    auto body = new QWidget();
    body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto layout = new QVBoxLayout(body);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addWidget(make_description_container(model));
    if(!model.m_tag.m_values.empty()) {
      layout->addWidget(make_values_container(model.m_tag.m_values));
    }
    return body;
  }

  auto make_scroll_box(const OrderFieldInfoTip::Model& model) {
    auto scroll_box = new ScrollBox(make_scroll_box_body(model));
    scroll_box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    scroll_box->setFixedWidth(scale_width(280));
    scroll_box->setMaximumHeight(scale_height(240));
    return scroll_box;
  }

  auto make_prerequisite_container(const QString& prerequisites) {
    auto header = make_label(QObject::tr("Prerequisites"));
    auto header_style = get_style(*header);
    header_style.get(ReadOnly()).
      set(FontSize(scale_height(10)));
    set_style(*header, std::move(header_style));
    auto prerequisites_label = new TextAreaBox(prerequisites);
    prerequisites_label->set_read_only(true);
    set_style(*prerequisites_label, PREREQUISITES_STYLE());
    auto container = new QWidget();
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto layout = new QVBoxLayout(container);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addWidget(header);
    layout->addWidget(prerequisites_label);
    auto box = new Box(container);
    box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto box_style = get_style(*box);
    box_style.get(Any()).
      set(BorderTopColor(QColor(0xE0E0E0))).
      set(BorderTopSize(scale_height(1))).
      set(padding(scale_width(18)));
    set_style(*box, std::move(box_style));
    return box;
  }
}

QString OrderFieldInfoTip::Model::get_prerequisites() const {
  auto text = QString();
  for(auto tag_index = std::size_t(0); tag_index < m_prerequisites.size();
      ++tag_index) {
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
  setFixedWidth(scale_width(280));
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  layout->addWidget(make_scroll_box(model));
  if(!model.m_prerequisites.empty()) {
    layout->addWidget(make_prerequisite_container(model.get_prerequisites()));
  }
  auto tip = new InfoTip(this, parent);
  tip->set_interactive(true);
}
