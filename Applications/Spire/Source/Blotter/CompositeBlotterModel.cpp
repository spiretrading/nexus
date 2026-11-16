#include "Spire/Blotter/CompositeBlotterModel.hpp"

using namespace Spire;

CompositeBlotterModel::CompositeBlotterModel(
  std::shared_ptr<TextModel> name) noexcept
  : m_name(std::move(name)) {}

const std::shared_ptr<TextModel>& CompositeBlotterModel::get_name() const {
  return m_name;
}
