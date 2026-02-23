#ifndef SPIRE_TICKER_INFO_COMPLETER_HPP
#define SPIRE_TICKER_INFO_COMPLETER_HPP
#include <memory>
#include <QCompleter>
#include "Spire/Utilities/Utilities.hpp"

class QSortFilterProxyModel;

namespace Spire {

  /** Implements auto-completion for TickerInfo items. */
  class TickerInfoCompleter : public QCompleter {
    public:

      /**
       * Constructs a TickerInfoCompleter.
       * @param parent The parent object.
       */
      TickerInfoCompleter(QObject* parent = nullptr);

      /**
       * Constructs a TickerInfoCompleter.
       * @param model The model populating this completer.
       * @param parent The parent object.
       */
      TickerInfoCompleter(TickerInfoModel* model, QObject* parent = nullptr);

      ~TickerInfoCompleter() override;

      QStringList splitPath(const QString& s) const override;
      QAbstractItemModel* sourceModel() const;
      void setSourceModel(TickerInfoModel* source);

    private:
      std::unique_ptr<QSortFilterProxyModel> m_proxyModel;

      void Initialize();
      void UpdateProxyModel() const;
  };
}

#endif
