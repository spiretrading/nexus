#include "Spire/Spire/AnyQueryModel.hpp"

using namespace boost;
using namespace Spire;

optional<std::any> AnyQueryModel::parse(const QString& query) {
  return m_source->parse(query);
}

QtPromise<std::vector<std::any>> AnyQueryModel::submit(const QString& query) {
  return m_source->submit(query);
}
