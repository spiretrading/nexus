#ifndef SPIRE_REGION_COMPLETER_HPP
#define SPIRE_REGION_COMPLETER_HPP
#include <memory>
#include <QCompleter>

class QSortFilterProxyModel;

namespace Spire {
  class RegionQueryModel;

  /** Implements auto-completion for Region values. */
  class RegionCompleter : public QCompleter {
    public:

      /**
       * Constructs a RegionCompleter.
       * @param model The model populating this completer.
       * @param parent The parent object.
       */
      explicit RegionCompleter(
        RegionQueryModel* model, QObject* parent = nullptr);

      ~RegionCompleter() override;

      QStringList splitPath(const QString& s) const override;

    private:
      std::unique_ptr<QSortFilterProxyModel> m_proxyModel;
  };
}

#endif
