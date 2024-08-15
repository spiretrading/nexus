#include "Spire/SignIn/Track.hpp"

using namespace Spire;

const QString& Spire::to_text(Track track) {
  if(track == Track::CLASSIC) {
    static auto text = QObject::tr("Classic");
    return text;
  } else if(track == Track::CURRENT) {
    static auto text = QObject::tr("Current");
    return text;
  } else if(track == Track::PREVIEW) {
    static auto text = QObject::tr("Preview");
    return text;
  }
  throw std::runtime_error("Unknown track.");
}

const QString& Spire::get_description(Track track) {
  if(track == Track::CLASSIC) {
    static auto text = QObject::tr("Legacy version.");
    return text;
  } else if(track == Track::CURRENT) {
    static auto text = QObject::tr("Latest stable release.");
    return text;
  } else if(track == Track::PREVIEW) {
    static auto text = QObject::tr("Preview upcoming features.");
    return text;
  }
  throw std::runtime_error("Unknown track.");
}
