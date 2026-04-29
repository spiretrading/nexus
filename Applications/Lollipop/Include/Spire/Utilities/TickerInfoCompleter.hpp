#ifndef SPIRE_TICKERINFOCOMPLETER_HPP
#define SPIRE_TICKERINFOCOMPLETER_HPP
#include <memory>
#include <QCompleter>
#include "Spire/Utilities/Utilities.hpp"

class QSortFilterProxyModel;

namespace Spire {

  /*! \class TickerInfoCompleter
      \brief Implements auto-completion for TickerInfo items.
   */
  class TickerInfoCompleter : public QCompleter {
    public:

      //! Constructs a TickerInfoCompleter.
      /*!
        \param parent The parent object.
      */
      TickerInfoCompleter(QObject* parent = nullptr);

      //! Constructs a TickerInfoCompleter.
      /*!
        \param model The model populating this completer.
        \param parent The parent object.
      */
      TickerInfoCompleter(TickerInfoModel* model,
        QObject* parent = nullptr);

      virtual ~TickerInfoCompleter();

      virtual QStringList splitPath(const QString& s) const;

      QAbstractItemModel* sourceModel() const;

      void setSourceModel(TickerInfoModel* source);

    private:
      std::unique_ptr<QSortFilterProxyModel> m_proxyModel;

      void Initialize();
      void UpdateProxyModel() const;
  };
}

#endif
