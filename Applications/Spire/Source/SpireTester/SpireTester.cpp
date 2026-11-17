#include "Spire/SpireTester/SpireTester.hpp"

using namespace Spire::Tests;

namespace {
  char** prepare_application(int& count) {
    static char name[] = "SpireTester";
    static char platform_option[] = "-platform";
    static char platform[] = "offscreen";
    static char* arguments[4];
#ifdef _WIN32
    if(qEnvironmentVariableIsEmpty("QT_QPA_FONTDIR")) {
      qputenv("QT_QPA_FONTDIR", SPIRE_TEST_FONT_DIRECTORY);
    }
#endif
    qputenv("QT_FONT_DPI", "96");
    arguments[0] = name;
    arguments[1] = platform_option;
    arguments[2] = platform;
    arguments[3] = nullptr;
    count = 3;
    return arguments;
  }
}

UiTestFixture::UiTestFixture()
  : m_application(m_count, prepare_application(m_count)) {}

UiTestFixture::~UiTestFixture() {
  QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}
