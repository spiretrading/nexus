#include "Spire/KeyBindings/AdditionalTagsBox.hpp"
#include "Spire/KeyBindings/AdditionalTagsWindow.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto to_text(const std::vector<AdditionalTag>& tags,
      const AdditionalTagDatabase& additional_tags,
      const Destination& destination, const Region& region) {
    auto label = QString();
    for(auto& tag : tags) {
      auto schema = find(additional_tags, destination, region, tag.m_key);
      if(schema) {
        if(!label.isEmpty()) {
          label.append(", ");
        }
        label.append(QString::fromStdString(schema->get_name()));
      }
    }
    return label;
  }
}

AdditionalTagsBox::AdditionalTagsBox(
    std::shared_ptr<AdditionalTagsModel> current,
    AdditionalTagDatabase additional_tags,
    std::shared_ptr<DestinationModel> destination,
    std::shared_ptr<RegionModel> region, QWidget* parent)
    : QWidget(parent),
      m_additional_tags(std::move(additional_tags)),
      m_destination(std::move(destination)),
      m_destination_connection(m_destination->connect_update_signal(
        std::bind_front(&AdditionalTagsBox::on_destination, this))),
      m_region(std::move(region)),
      m_region_connection(m_region->connect_update_signal(
        std::bind_front(&AdditionalTagsBox::on_region, this))),
      m_current(std::move(current)),
      m_is_read_only(false),
      m_click_observer(*this) {
  m_tags_text = make_transform_value_model(m_current,
    [=] (const auto& current) {
      return ::to_text(
        current, m_additional_tags, m_destination->get(), m_region->get());
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

void AdditionalTagsBox::update_current(
    const Destination& destination, const Region& region) {
  auto current = m_current->get();
  auto tags = Spire::find(m_additional_tags, destination, region);
  auto erasures = std::erase_if(current, [&] (const auto& current) {
    auto i = std::find_if(tags.begin(), tags.end(), [&] (const auto& tag) {
      return tag->get_key() == current.m_key;
    });
    return i == tags.end() || !(*i)->test(current);
  });
  if(erasures != 0) {
    m_current->set(current);
  }
}

void AdditionalTagsBox::on_destination(const Destination& destination) {
  update_current(destination, m_region->get());
}

void AdditionalTagsBox::on_region(const Region& region) {
  update_current(m_destination->get(), region);
}

void AdditionalTagsBox::on_click() {
  if(m_is_read_only) {
    return;
  }
  auto window = new AdditionalTagsWindow(
    m_current, m_additional_tags, m_destination, m_region, this);
  window->setAttribute(Qt::WA_DeleteOnClose);
  window->setWindowModality(Qt::WindowModal);
  window->show();
}
