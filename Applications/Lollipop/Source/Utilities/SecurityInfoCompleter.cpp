#include "Spire/Utilities/SecurityInfoCompleter.hpp"
#include <QSortFilterProxyModel>
#include <QStringList>
#include "Spire/Utilities/SecurityInfoModel.hpp"

using namespace Beam;
using namespace Spire;

SecurityInfoCompleter::SecurityInfoCompleter(QObject* parent)
    : QCompleter(parent) {
  Initialize();
}

SecurityInfoCompleter::SecurityInfoCompleter(SecurityInfoModel* model,
    QObject* parent)
    : QCompleter(parent) {
  Initialize();
  setSourceModel(model);
}

SecurityInfoCompleter::~SecurityInfoCompleter() {}

QStringList SecurityInfoCompleter::splitPath(const QString& s) const {
  UpdateProxyModel();
  QRegExp regexp;
  if(s.isEmpty()) {
    regexp = QRegExp("^\\$\\:");
  } else {
    regexp = QRegExp("^" + QRegExp::escape(s));
  }
  regexp.setCaseSensitivity(caseSensitivity());
  m_proxyModel->setFilterRegExp(regexp);
  return QStringList();
}

QAbstractItemModel* SecurityInfoCompleter::sourceModel() const {
  return m_proxyModel->sourceModel();
}

void SecurityInfoCompleter::setSourceModel(SecurityInfoModel* source) {
  QAbstractItemModel* model = source;
  m_proxyModel->setSourceModel(source);
  if(source != nullptr && model->parent() == this) {
    source->setParent(m_proxyModel.get());
  }
  model = m_proxyModel.get();
  setModel(model);
}

void SecurityInfoCompleter::Initialize() {
  m_proxyModel = std::make_unique<QSortFilterProxyModel>(nullptr);
}

void SecurityInfoCompleter::UpdateProxyModel() const {
  m_proxyModel->setFilterCaseSensitivity(caseSensitivity());
  m_proxyModel->setFilterKeyColumn(-1);
}
