#include "Spire/Utilities/TickerInfoCompleter.hpp"
#include <QSortFilterProxyModel>
#include <QStringList>
#include "Spire/Utilities/TickerInfoModel.hpp"

using namespace Beam;
using namespace Spire;

TickerInfoCompleter::TickerInfoCompleter(QObject* parent)
    : QCompleter(parent) {
  Initialize();
}

TickerInfoCompleter::TickerInfoCompleter(TickerInfoModel* model,
    QObject* parent)
    : QCompleter(parent) {
  Initialize();
  setSourceModel(model);
}

TickerInfoCompleter::~TickerInfoCompleter() {}

QStringList TickerInfoCompleter::splitPath(const QString& s) const {
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

QAbstractItemModel* TickerInfoCompleter::sourceModel() const {
  return m_proxyModel->sourceModel();
}

void TickerInfoCompleter::setSourceModel(TickerInfoModel* source) {
  QAbstractItemModel* model = source;
  m_proxyModel->setSourceModel(source);
  if(source != nullptr && model->parent() == this) {
    source->setParent(m_proxyModel.get());
  }
  model = m_proxyModel.get();
  setModel(model);
}

void TickerInfoCompleter::Initialize() {
  m_proxyModel = std::make_unique<QSortFilterProxyModel>(nullptr);
}

void TickerInfoCompleter::UpdateProxyModel() const {
  m_proxyModel->setFilterCaseSensitivity(caseSensitivity());
  m_proxyModel->setFilterKeyColumn(-1);
}
