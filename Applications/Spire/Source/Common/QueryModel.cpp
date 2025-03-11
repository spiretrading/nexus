#include "Spire/Spire/QueryModel.hpp"

using namespace Spire;

std::any AnyQueryModel::parse(const QString& query) {
  return parse_any(query);
}

QtPromise<std::vector<std::any>> AnyQueryModel::submit(const QString& query) {
  return submit_any(query);
}
