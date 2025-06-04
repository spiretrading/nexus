#include "Spire/Playback/PlaybackSpeedBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace Spire;

namespace {
  auto to_text(const Decimal& value) {
    return QString("%1x").arg(QString::fromStdString(value.str(
      Decimal::backend_type::cpp_dec_float_digits10, std::ios_base::dec)));
  }

  PlaybackSpeedBox::Settings setup() {
    static auto settings = [] {
      auto settings = PlaybackSpeedBox::Settings(
        [] (auto& value) {
          return to_text(value);
        },
        [] (auto& value) {
          return make_label(to_text(value));
        });
      settings.m_cases = std::make_shared<ArrayListModel<Decimal>>(
        std::vector<Decimal>{0.1, 0.5, 1, 2, 5, 10, 100});
      return settings;
    }();
    return settings;
  }
}

PlaybackSpeedBox* Spire::make_playback_speed_box(QWidget* parent) {
  return make_playback_speed_box(
    std::make_shared<LocalValueModel<Decimal>>(Decimal(1)), parent);
}

PlaybackSpeedBox* Spire::make_playback_speed_box(
    std::shared_ptr<ValueModel<Decimal>> current, QWidget* parent) {
  auto settings = setup();
  settings.m_current = std::move(current);
  return new PlaybackSpeedBox(std::move(settings), parent);
}
