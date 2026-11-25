#include "Spire/SpireTester/SpireTester.hpp"
#include <QFontDatabase>
#include <QtPlugin>

#if defined(_WIN32) && defined(QT_STATIC)
  Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
  Q_IMPORT_PLUGIN(QGifPlugin)
  Q_IMPORT_PLUGIN(QSvgIconPlugin)
#endif

int QT_MANGLE_NAMESPACE(qInitResources_Resources)();

using namespace Spire::Tests;

namespace {
  void initialize_resources() {
    ::QT_MANGLE_NAMESPACE(qInitResources_Resources)();
    auto fonts = QStringList();
    fonts << ":/Fonts/Roboto/Roboto-Black.ttf" <<
      ":/Fonts/Roboto/Roboto-BlackItalic.ttf" <<
      ":/Fonts/Roboto/Roboto-Bold.ttf" <<
      ":/Fonts/Roboto/Roboto-BoldItalic.ttf" <<
      ":/Fonts/Roboto/RobotoCondensed-Bold.ttf" <<
      ":/Fonts/Roboto/RobotoCondensed-BoldItalic.ttf" <<
      ":/Fonts/Roboto/RobotoCondensed-Italic.ttf" <<
      ":/Fonts/Roboto/RobotoCondensed-Light.ttf" <<
      ":/Fonts/Roboto/RobotoCondensed-LightItalic.ttf" <<
      ":/Fonts/Roboto/RobotoCondensed-Regular.ttf" <<
      ":/Fonts/Roboto/RobotoMono-Regular.ttf" <<
      ":/Fonts/Roboto/Roboto-Italic.ttf" <<
      ":/Fonts/Roboto/Roboto-Light.ttf" <<
      ":/Fonts/Roboto/Roboto-LightItalic.ttf" <<
      ":/Fonts/Roboto/Roboto-Medium.ttf" <<
      ":/Fonts/Roboto/Roboto-MediumItalic.ttf" <<
      ":/Fonts/Roboto/Roboto-Regular.ttf" <<
      ":/Fonts/Roboto/Roboto-Thin.ttf" <<
      ":/Fonts/Roboto/Roboto-ThinItalic.ttf";
    for(auto& font : fonts) {
      QFontDatabase::addApplicationFont(font);
    }
    auto font = QApplication::font();
    font.setStyleStrategy(QFont::PreferAntialias);
    QApplication::setFont(font);
  }

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
    : m_application(m_count, prepare_application(m_count)) {
  initialize_resources();
}

UiTestFixture::~UiTestFixture() {
  QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}
