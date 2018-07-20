#include "spire/spire/export_model.hpp"
#include "spire/ui/custom_qt_variants.hpp"

using namespace spire;

void spire::export_model_as_csv(const QAbstractItemModel& model,
    std::ostream& out) {
  custom_variant_item_delegate delegate;
  for(auto i = 0; i < model.columnCount(); ++i) {
    auto title = delegate.displayText(model.headerData(i, Qt::Horizontal,
      Qt::DisplayRole), QLocale());
    if(i != 0) {
      out << ',';
    }
    out << '\"' << title.toStdString() << '\"';
  }
  out << '\n';
  for(auto i = 0; i < model.rowCount(); ++i) {
    if(i != 0) {
      out << '\n';
    }
    for(auto j = 0; j < model.columnCount(); ++j) {
      auto index = model.index(i, j);
      auto value_variant = model.data(index, Qt::DisplayRole);
      auto value_text = delegate.displayText(model.data(index, Qt::DisplayRole),
        QLocale());
      if(value_variant.type() == QVariant::String) {
        value_text = "\"" + value_text + "\"";
      } else if(value_variant.type() == QVariant::Double ||
          value_variant.type() == QVariant::Int ||
          value_variant.type() == QVariant::LongLong ||
          value_variant.type() == QVariant::UInt ||
          value_variant.type() == QVariant::ULongLong) {
        value_text.remove(QChar(','));
      }
      if(j != 0) {
        out << ',';
      }
      out << value_text.toStdString();
    }
  }
}
