#ifndef SPIRE_UI_VIEWER_QUERY_PROFILE_BUILDERS_HPP
#define SPIRE_UI_VIEWER_QUERY_PROFILE_BUILDERS_HPP
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/OpenFilterPanel.hpp"
#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /**
   * Returns the display name of an open filter mode.
   * @tparam T The type of tag list box used by the filter panel.
   * @param mode The filter mode to display.
   */
  template<typename T>
  auto to_string(typename OpenFilterPanel<T>::Mode mode) {
    using Mode = typename OpenFilterPanel<T>::Mode;
    if(mode == Mode::INCLUDE) {
      return QString("Include");
    }
    return QString("Exclude");
  }

  /**
   * Builds a profile for a tag combo box.
   * @tparam F The tag combo box factory type.
   * @param name The profile name displayed in the viewer.
   * @param make_box Constructs a tag combo box for the profile.
   */
  template<typename F>
  auto setup_tag_combo_box_profile(const QString& name, F make_box) {
    auto properties = std::vector<std::shared_ptr<UiProperty>>();
    populate_widget_properties(properties);
    properties.push_back(make_standard_property<QString>("placeholder"));
    properties.push_back(make_standard_property("read_only", false));
    return UiProfile(name, properties, [=] (UiProfile& profile) {
      auto box = make_box();
      using T = typename std::decay_t<decltype(*box)>::Type;
      box->setMinimumWidth(scale_width(112));
      apply_widget_properties(box, profile.get_properties());
      auto& placeholder =
        get<QString>("placeholder", profile.get_properties());
      placeholder.connect_changed_signal([=] (const auto& placeholder) {
        box->set_placeholder(placeholder);
      });
      auto& read_only = get<bool>("read_only", profile.get_properties());
      read_only.connect_changed_signal(
        std::bind_front(&std::decay_t<decltype(*box)>::set_read_only, box));
      auto current_filter_slot =
        profile.make_event_slot<QString>(QString::fromUtf8("Current"));
      auto print_current = [=] {
        auto result = QString();
        for(auto i = 0; i < box->get_current()->get_size(); ++i) {
          result += QString("[%1] ").arg(to_text(box->get_current()->get(i)));
        }
        current_filter_slot(result);
      };
      box->get_current()->connect_operation_signal(
        [=] (const auto& operation) {
          visit(operation,
            [=] (const typename ListModel<T>::AddOperation&) {
              print_current();
            },
            [=] (const typename ListModel<T>::RemoveOperation&) {
              print_current();
            });
        });
      auto submit_filter_slot =
        profile.make_event_slot<QString>(QString::fromUtf8("Submit"));
      box->connect_submit_signal([=] (const auto& submission) {
        auto result = QString();
        for(auto i = 0; i < submission->get_size(); ++i) {
          result += QString("[%1] ").arg(to_text(submission->get(i)));
        }
        submit_filter_slot(result);
      });
      return box;
    });
  }

  /**
   * Builds a profile for an open filter panel.
   * @tparam F The open filter panel factory type.
   * @param name The profile name displayed in the viewer.
   * @param make_panel Constructs an open filter panel for the profile.
   */
  template<typename F>
  auto setup_open_filter_panel_profile(const QString& name, F make_panel) {
    auto properties = std::vector<std::shared_ptr<UiProperty>>();
    populate_widget_properties(properties);
    return UiProfile(name, properties, [=] (UiProfile& profile) {
      auto filter_panel = make_panel();
      using TagListBox =
        std::decay_t<decltype(filter_panel->get_tag_list_box())>;
      apply_widget_properties(filter_panel, profile.get_properties());
      auto submit_slot = profile.make_event_slot<QString>("Submit");
      filter_panel->connect_submit_signal(
        [=] (const auto& submission, auto mode) {
          auto items = QString();
          for(auto i = 0; i < submission->get_size(); ++i) {
            items += to_text(submission->get(i)) + " ";
          }
          submit_slot(QString("Mode:%1 [%2]").
            arg(to_string<TagListBox>(mode)).arg(items));
        });
      return filter_panel;
    });
  }

  /**
   * Builds a profile for a query-backed combo box.
   * @tparam Source The query model type.
   * @tparam Extract The type of the callable that extracts a selection.
   * @tparam MakeBox The combo box factory type.
   * @param name The profile name displayed in the viewer.
   * @param default_current The initial text to look up in the query model.
   * @param source The model that resolves text to query results.
   * @param extract Maps a query result to the selected value.
   * @param make_box Constructs the box from the query and selection models.
   */
  template<typename Source, typename Extract, typename MakeBox>
  UiProfile setup_combo_box_profile(const QString& name,
      const QString& default_current, std::shared_ptr<Source> source,
      Extract extract, MakeBox make_box) {
    using Type = std::invoke_result_t<Extract, typename Source::Type>;
    auto properties = std::vector<std::shared_ptr<UiProperty>>();
    populate_widget_properties(properties);
    properties.push_back(
      make_standard_property<QString>("current", default_current));
    properties.push_back(make_standard_property<QString>("placeholder"));
    properties.push_back(make_standard_property("read_only", false));
    return UiProfile(name, properties, [=] (UiProfile& profile) {
      auto& current = get<QString>("current", profile.get_properties());
      auto current_value = [&] {
        if(auto value = source->parse(current.get())) {
          return extract(*value);
        }
        return Type();
      }();
      auto current_model = make_local_value_model(current_value);
      auto box = make_box(source, current_model);
      using Box = std::decay_t<decltype(*box)>;
      box->setMinimumWidth(scale_width(112));
      apply_widget_properties(box, profile.get_properties());
      auto current_connection = box->get_current()->connect_update_signal(
        profile.make_event_slot<Type>("Current"));
      current.connect_changed_signal([=] (const auto& current) {
        if(auto value = source->parse(current)) {
          box->get_current()->set(extract(*value));
        } else {
          auto current_blocker =
            boost::signals2::shared_connection_block(current_connection);
          box->get_current()->set(Type());
        }
      });
      auto& placeholder =
        get<QString>("placeholder", profile.get_properties());
      placeholder.connect_changed_signal([=] (const auto& placeholder) {
        box->set_placeholder(placeholder);
      });
      link(&Box::is_read_only, &Box::set_read_only,
        *box, get<bool>("read_only", profile.get_properties()));
      box->connect_submit_signal(profile.make_event_slot<Type>("Submit"));
      return box;
    });
  }
}

#endif
