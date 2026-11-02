#include <Beam/Utilities/DoctestMain.hpp>
#include <QtPlugin>

#if defined(_WIN32) && defined(QT_STATIC)
  Q_IMPORT_PLUGIN(QOffscreenIntegrationPlugin)
#endif

DOCTEST_MAIN()
