#ifndef SPIRE_SECURITYINFOCOMPLETER_HPP
#define SPIRE_SECURITYINFOCOMPLETER_HPP
#include <memory>
#include <QCompleter>
#include "Spire/Utilities/Utilities.hpp"

class QSortFilterProxyModel;

namespace Spire {

  /*! \class SecurityInfoCompleter
      \brief Implements auto-completion for SecurityInfo items.
   */
  class SecurityInfoCompleter : public QCompleter {
    public:

      //! Constructs a SecurityInfoCompleter.
      /*!
        \param parent The parent object.
      */
      SecurityInfoCompleter(QObject* parent = nullptr);

      //! Constructs a SecurityInfoCompleter.
      /*!
        \param model The model populating this completer.
        \param parent The parent object.
      */
      SecurityInfoCompleter(SecurityInfoModel* model,
        QObject* parent = nullptr);

      virtual ~SecurityInfoCompleter();

      virtual QStringList splitPath(const QString& s) const;

      QAbstractItemModel* sourceModel() const;

      void setSourceModel(SecurityInfoModel* source);

    private:
      std::unique_ptr<QSortFilterProxyModel> m_proxyModel;

      void Initialize();
      void UpdateProxyModel() const;
  };
}

#endif
