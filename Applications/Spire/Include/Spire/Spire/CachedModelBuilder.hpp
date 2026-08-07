#ifndef SPIRE_CACHED_MODEL_BUILDER_HPP
#define SPIRE_CACHED_MODEL_BUILDER_HPP
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <QTimer>

namespace Spire {
namespace Details {
  class ExpiryTimer {
    public:
      static constexpr auto EXPIRY_INTERVAL = std::chrono::milliseconds(1000);

      static ExpiryTimer& get_instance();
      std::int64_t add(std::function<void ()> callback);
      void remove(std::int64_t id);

    private:
      QTimer m_timer;
      std::unordered_map<std::int64_t, std::function<void ()>> m_callbacks;
      std::int64_t m_next_id;

      ExpiryTimer();
      ExpiryTimer(const ExpiryTimer&) = delete;
      ExpiryTimer& operator =(const ExpiryTimer&) = delete;
  };
}

  /**
   * Builds models keyed by an index and caches them, keeping each model alive
   * for a grace period after its last external reference is released.
   * @param I The type of index.
   * @param M The type of model to build.
   */
  template<typename I, typename M>
  class CachedModelBuilder {
    public:

      /** The type of index. */
      using Index = I;

      /** The type of model to build. */
      using Model = M;

      /**
       * The type of function used to build a model for an index.
       * @param index The index to build a model for.
       * @return The newly built model associated with the <i>index</i>.
       */
      using ModelBuilder =
        std::function<std::unique_ptr<Model> (const Index& index)>;

      /** The grace period used when no expiry is specified. */
      inline static const auto DEFAULT_EXPIRY = boost::posix_time::seconds(3);

      /**
       * Constructs a CachedModelBuilder using the default grace period.
       * @param builder The function used to build models.
       */
      explicit CachedModelBuilder(ModelBuilder builder);

      /**
       * Constructs a CachedModelBuilder.
       * @param builder The function used to build a models.
       * @param expiry How long a model is kept alive after its last reference
       *        is released.
       */
      CachedModelBuilder(
        ModelBuilder builder, boost::posix_time::time_duration expiry);

      ~CachedModelBuilder();

      /**
       * Returns the model associated with an index.
       * @param index The index whose model to return.
       * @return The shared model associated with the <i>index</i>.
       */
      std::shared_ptr<Model> load(const Index& index);

    private:
      struct State {
        ModelBuilder m_builder;
        std::chrono::steady_clock::duration m_expiry;
        std::unordered_map<Index, std::weak_ptr<Model>> m_models;
        std::unordered_map<Index, std::pair<std::unique_ptr<Model>,
          std::chrono::steady_clock::time_point>> m_pending;

        State(ModelBuilder builder, boost::posix_time::time_duration expiry);

        void park(const Index& index, Model* model);
        void expire();
      };
      std::shared_ptr<State> m_state;
      std::int64_t m_callback_id;

      CachedModelBuilder(const CachedModelBuilder&) = delete;
      CachedModelBuilder& operator =(const CachedModelBuilder&) = delete;
  };

  template<typename I, typename M>
  CachedModelBuilder<I, M>::State::State(
    ModelBuilder builder, boost::posix_time::time_duration expiry)
    : m_builder(std::move(builder)),
      m_expiry(std::chrono::microseconds(expiry.total_microseconds())) {}

  template<typename I, typename M>
  void CachedModelBuilder<I, M>::State::park(const Index& index, Model* model) {
    m_models.erase(index);
    m_pending[index] = std::pair(std::unique_ptr<Model>(model),
      std::chrono::steady_clock::now() + m_expiry);
  }

  template<typename I, typename M>
  void CachedModelBuilder<I, M>::State::expire() {
    auto now = std::chrono::steady_clock::now();
    std::erase_if(m_pending, [&] (const auto& entry) {
      return entry.second.second <= now;
    });
  }

  template<typename I, typename M>
  CachedModelBuilder<I, M>::CachedModelBuilder(ModelBuilder builder)
    : CachedModelBuilder(std::move(builder), DEFAULT_EXPIRY) {}

  template<typename I, typename M>
  CachedModelBuilder<I, M>::CachedModelBuilder(
      ModelBuilder builder, boost::posix_time::time_duration expiry)
      : m_state(std::make_shared<State>(std::move(builder), expiry)) {
    auto state = std::weak_ptr(m_state);
    m_callback_id = Details::ExpiryTimer::get_instance().add([state] {
      if(auto self = state.lock()) {
        self->expire();
      }
    });
  }

  template<typename I, typename M>
  CachedModelBuilder<I, M>::~CachedModelBuilder() {
    Details::ExpiryTimer::get_instance().remove(m_callback_id);
  }

  template<typename I, typename M>
  std::shared_ptr<M> CachedModelBuilder<I, M>::load(const Index& index) {
    if(auto i = m_state->m_models.find(index); i != m_state->m_models.end()) {
      if(auto model = i->second.lock()) {
        return model;
      }
    }
    auto instance = std::unique_ptr<Model>();
    if(auto i = m_state->m_pending.find(index); i != m_state->m_pending.end()) {
      instance = std::move(i->second.first);
      m_state->m_pending.erase(i);
    } else {
      instance = m_state->m_builder(index);
    }
    auto state = std::weak_ptr(m_state);
    auto model = std::shared_ptr<Model>(
      instance.release(), [state, index] (Model* p) {
        if(auto self = state.lock()) {
          self->park(index, p);
        } else {
          delete p;
        }
      });
    m_state->m_models[index] = model;
    return model;
  }
}

#endif
