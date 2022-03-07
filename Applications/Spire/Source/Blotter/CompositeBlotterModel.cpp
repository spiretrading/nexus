#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Blotter/CompositeBlotterStatusModel.hpp"
#include "Spire/Blotter/LocalBlotterPositionsModel.hpp"

using namespace Spire;

CompositeBlotterModel::CompositeBlotterModel(std::shared_ptr<TextModel> name,
    std::shared_ptr<BlotterPositionsModel> positions,
    std::shared_ptr<BlotterStatusModel> status)
  : m_name(std::move(name)),
    m_positions(std::move(positions)),
    m_status(std::move(status)) {}

std::shared_ptr<TextModel> CompositeBlotterModel::get_name() {
  return m_name;
}

std::shared_ptr<BlotterPositionsModel> CompositeBlotterModel::get_positions() {
  return m_positions;
}

std::shared_ptr<BlotterStatusModel> CompositeBlotterModel::get_status() {
  return m_status;
}

std::shared_ptr<CompositeBlotterModel> Spire::make_local_blotter_model() {
  return std::make_shared<CompositeBlotterModel>(
    std::make_shared<LocalTextModel>(),
    std::make_shared<LocalBlotterPositionsModel>(),
    make_local_blotter_status_model());
}
