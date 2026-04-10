#include "Spire/Utilities/ScopeCompleter.hpp"
#include <QSortFilterProxyModel>
#include <QStringList>
#include "Spire/Utilities/ScopeQueryModel.hpp"

using namespace Beam;
using namespace Spire;

ScopeCompleter::ScopeCompleter(ScopeQueryModel* model, QObject* parent)
    : QCompleter(parent),
      m_proxyModel(std::make_unique<QSortFilterProxyModel>()) {
  m_proxyModel->setSourceModel(model);
  setModel(m_proxyModel.get());
}

ScopeCompleter::~ScopeCompleter() = default;

QStringList ScopeCompleter::splitPath(const QString& s) const {
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
