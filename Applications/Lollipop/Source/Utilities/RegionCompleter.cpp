#include "Spire/Utilities/RegionCompleter.hpp"
#include <QSortFilterProxyModel>
#include <QStringList>
#include "Spire/Utilities/RegionQueryModel.hpp"

using namespace Beam;
using namespace Spire;

RegionCompleter::RegionCompleter(RegionQueryModel* model, QObject* parent)
    : QCompleter(parent),
      m_proxyModel(std::make_unique<QSortFilterProxyModel>()) {
  m_proxyModel->setSourceModel(model);
  setModel(m_proxyModel.get());
}

RegionCompleter::~RegionCompleter() = default;

QStringList RegionCompleter::splitPath(const QString& s) const {
  m_proxyModel->setFilterCaseSensitivity(caseSensitivity());
  m_proxyModel->setFilterKeyColumn(-1);
  auto regex = [&] {
    if(s.isEmpty()) {
      return QRegExp("^\\$\\:");
    } else {
      return QRegExp("^" + QRegExp::escape(s));
    }
  }();
  regex.setCaseSensitivity(caseSensitivity());
  m_proxyModel->setFilterRegExp(regex);
  return QStringList();
}
