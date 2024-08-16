#include "Spire/Utilities/ExportModel.hpp"
#include <QAbstractItemModel>
#include "Spire/UI/CustomQtVariants.hpp"

using namespace Beam;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

void Spire::ExportModelAsCsv(UserProfile& userProfile,
    const QAbstractItemModel& model, ostream& out) {
  auto delegate = CustomVariantItemDelegate(Ref(userProfile));
  for(int i = 0; i < model.columnCount(); ++i) {
    QString columnTitle = delegate.displayText(
      model.headerData(i, Qt::Horizontal, Qt::DisplayRole), QLocale());
    if(i != 0) {
      out << ",";
    }
    out << "\"" << columnTitle.toStdString() << "\"";
  }
  out << endl;
  for(int i = 0; i < model.rowCount(); ++i) {
    if(i != 0) {
      out << endl;
    }
    for(int j = 0; j < model.columnCount(); ++j) {
      QModelIndex index = model.index(i, j);
      QVariant valueVariant = model.data(index, Qt::DisplayRole);
      QString valueText = delegate.displayText(
        model.data(index, Qt::DisplayRole), QLocale());
      if(valueVariant.type() == QVariant::String) {
        valueText = "\"" + valueText + "\"";
      } else if(valueVariant.type() == QVariant::Double ||
          valueVariant.type() == QVariant::Int ||
          valueVariant.type() == QVariant::LongLong ||
          valueVariant.type() == QVariant::UInt ||
          valueVariant.type() == QVariant::ULongLong) {
        valueText.remove(QChar(','));
      }
      if(j != 0) {
        out << ",";
      }
      out << valueText.toStdString();
    }
  }
}
