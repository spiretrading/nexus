#include "Spire/KeyBindings/AdditionalTagsBox.hpp"
#include "Spire/KeyBindings/AdditionalTagsWindow.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

AdditionalTagsBox::AdditionalTagsBox(AdditionalTagDatabase additional_tags,
    std::shared_ptr<DestinationModel> destination,
    std::shared_ptr<RegionModel> region,
    std::shared_ptr<AdditionalTagsModel> current, QWidget* parent)
    : QWidget(parent),
      m_additional_tags(std::move(additional_tags)),
      m_destination(std::move(destination)),
      m_region(std::move(region)),
      m_current(std::move(current)),
      m_is_read_only(false),
      m_click_observer(*this) {
  m_tags_text = make_transform_value_model(m_current,
    [=] (const auto& current) {
      auto label = QString();
      for(auto& tag : current) {
        auto schema = Spire::find(
          m_additional_tags, m_destination->get(), m_region->get(), tag.m_key);
        if(schema) {
          if(!label.isEmpty()) {
            label.append(", ");
          }
          label.append(QString::fromStdString(schema->get_name()));
        }
      }
      return label;
    });
  m_label = make_label(m_tags_text);
  enclose(*this, *m_label);
  proxy_style(*this, *m_label);
  m_click_observer.connect_click_signal(
    std::bind_front(&AdditionalTagsBox::on_click, this));
}

const std::shared_ptr<AdditionalTagsModel>&
    AdditionalTagsBox::get_current() const {
  return m_current;
}

void AdditionalTagsBox::set_placeholder(const QString& placeholder) {
  m_label->set_placeholder(placeholder);
}

bool AdditionalTagsBox::is_read_only() const {
  return m_is_read_only;
}

void AdditionalTagsBox::set_read_only(bool is_read_only) {
  if(is_read_only == m_is_read_only) {
    return;
  }
  m_is_read_only = is_read_only;
}

connection AdditionalTagsBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return {};
}

void AdditionalTagsBox::on_click() {
  auto window = new AdditionalTagsWindow(this);
  window->setWindowModality(Qt::WindowModal);
  window->show();
}
