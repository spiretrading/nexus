#ifndef SPIRE_ANY_QUERY_MODEL_HPP
#define SPIRE_ANY_QUERY_MODEL_HPP
#include <any>
#include <concepts>
#include <memory>
#include "Spire/Spire/QueryModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Implements a QueryModel by wrapping a QueryModel with a type-safe generic
   * interface.
   */
  class AnyQueryModel : public QueryModel<std::any> {
    public:

      /**
       * Constructs an AnyQueryModel over an existing QueryModel.
       * @param source The QueryModel to wrap.
       */
      template<typename T>
      explicit AnyQueryModel(std::shared_ptr<T> source) requires
        std::derived_from<T, QueryModel<typename T::Type>>;

      boost::optional<std::any> parse(const QString& query) override;

      QtPromise<std::vector<std::any>> submit(const QString& query) override;

    private:
      struct VirtualQueryModel {
        virtual ~VirtualQueryModel() = default;
        virtual boost::optional<std::any> parse(const QString& query) = 0;
        virtual
          QtPromise<std::vector<std::any>> submit(const QString& query) = 0;
      };
      template<typename T>
      struct WrapperQueryModel final : VirtualQueryModel {
        std::shared_ptr<QueryModel<T>> m_source;

        WrapperQueryModel(std::shared_ptr<QueryModel<T>> source);
        boost::optional<std::any> parse(const QString& query) override;
        QtPromise<std::vector<std::any>> submit(const QString& query) override;
      };
      std::unique_ptr<VirtualQueryModel> m_source;
  };

  template<typename T>
  AnyQueryModel::AnyQueryModel(std::shared_ptr<T> source) requires
    std::derived_from<T, QueryModel<typename T::Type>>
    : m_source(std::make_unique<WrapperQueryModel<typename T::Type>>(
        std::move(source))) {}

  template<typename T>
  AnyQueryModel::WrapperQueryModel<T>::WrapperQueryModel(
    std::shared_ptr<QueryModel<T>> source)
    : m_source(std::move(source)) {}

  template<typename T>
  boost::optional<std::any>
      AnyQueryModel::WrapperQueryModel<T>::parse(const QString& query) {
    if(auto result = m_source->parse(query)) {
      return boost::optional<std::any>(*result);
    }
    return boost::none;
  }

  template<typename T>
  QtPromise<std::vector<std::any>>
      AnyQueryModel::WrapperQueryModel<T>::submit(const QString& query) {
    return m_source->submit(query).then([] (std::vector<T> result) {
      return std::vector<std::any>(std::make_move_iterator(result.begin()),
        std::make_move_iterator(result.end()));
    });
  }
}

#endif
