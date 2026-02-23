#ifndef SPIRE_SCOPE_COMPLETER_HPP
#define SPIRE_SCOPE_COMPLETER_HPP
#include <memory>
#include <QCompleter>

class QSortFilterProxyModel;

namespace Spire {
  class ScopeQueryModel;

  /** Implements auto-completion for Scopes. */
  class ScopeCompleter : public QCompleter {
    public:

      /**
       * Constructs a ScopeCompleter.
       * @param model The model populating this completer.
       * @param parent The parent object.
       */
      explicit ScopeCompleter(
        ScopeQueryModel* model, QObject* parent = nullptr);

      ~ScopeCompleter() override;
      QStringList splitPath(const QString& s) const override;

    private:
      std::unique_ptr<QSortFilterProxyModel> m_proxyModel;
  };
}

#endif
