#include <QTemporaryDir>
#include "Spire/KeyBindings/KeyBindingsProfile.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Nexus;
using namespace Spire;

TEST_SUITE("KeyBindingsProfile") {
  UI_TEST_CASE("duplicate_task_shortcuts") {
    auto directory = QTemporaryDir();
    REQUIRE(directory.isValid());
    auto path = std::filesystem::path(directory.path().toStdWString());
    auto bindings = KeyBindingsModel();
    auto& tasks = *bindings.get_order_task_arguments();
    auto add = [&] (QString name, Venue venue, QKeySequence key) {
      auto task = OrderTaskArguments();
      task.m_name = std::move(name);
      task.m_scope = Scope(venue);
      task.m_key = std::move(key);
      tasks.push(task);
    };
    auto key = QKeySequence(Qt::Key_F3);
    add("First", Venues::TSX, key);
    add("Other venue", Venues::ASX, key);
    add("Duplicate", Venues::TSX, key);
    add("Other key", Venues::TSX, Qt::Key_F4);
    save_key_bindings_profile(bindings, path);
    auto loaded = load_key_bindings_profile(path);
    auto& restored = *loaded->get_order_task_arguments();
    REQUIRE(restored.get_size() == tasks.get_size());
    for(auto i = 0; i < tasks.get_size(); ++i) {
      auto expected = tasks.get(i);
      if(i == 2) {
        expected.m_key = QKeySequence();
      }
      REQUIRE(restored.get(i) == expected);
    }
  }
}
