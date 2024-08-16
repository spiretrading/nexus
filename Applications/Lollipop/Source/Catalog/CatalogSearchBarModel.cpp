#include "Spire/Catalog/CatalogSearchBarModel.hpp"
#include "Spire/Catalog/CatalogEntry.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace std;

CatalogSearchBarModel::CatalogSearchBarModel()
    : m_mode(INPUT) {}

CatalogSearchBarModel::Mode CatalogSearchBarModel::GetMode() const {
  return m_mode;
}

const string& CatalogSearchBarModel::GetText() const {
  return m_text;
}

void CatalogSearchBarModel::SetCatalogEntries(
    const vector<const CatalogEntry*>& entries) {
  m_text.clear();
  for(auto i = entries.begin(); i != entries.end(); ++i) {
    if(i != entries.begin()) {
      m_text += ", ";
    }
    m_text += (*i)->GetName();
  }
  m_mode = CATALOG_ENTRY;
  m_updateSignal(m_mode, m_text);
}

void CatalogSearchBarModel::SetLabel(const string& label) {
  m_text = label;
  m_mode = LABEL;
  m_updateSignal(m_mode, m_text);
}

void CatalogSearchBarModel::SetInput(const std::string& input) {
  m_text = input;
  m_mode = INPUT;
  m_updateSignal(m_mode, m_text);
}

connection CatalogSearchBarModel::ConnectUpdateSignal(
    const UpdateSignal::slot_type& slot) const {
  return m_updateSignal.connect(slot);
}
