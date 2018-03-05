#include "spire/login/chroma_hash_widget.hpp"
#include <Beam/ServiceLocator/SessionEncryption.hpp>

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace spire;

chroma_hash_widget::chroma_hash_widget(QWidget* parent) {
  set_text({});
}

void chroma_hash_widget::set_text(const QString& text) {
  const auto COLOR_LENGTH = 6;
  auto hash = ComputeSHA(text.toStdString());
  for(auto i = 0; i < COMPONENTS; ++i) {
    auto color_name = "#" + hash.substr(COLOR_LENGTH * i, COLOR_LENGTH);
    m_colors[i] = QColor(QString::fromStdString(color_name));
  }
}
