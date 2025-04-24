#include "Spire/KeyBindings/OrderTaskArgumentsMatch.hpp"
#include <queue>

using namespace Spire;

namespace {
  class HopcroftKarp {
    public:
      HopcroftKarp(int left_size, int right_size)
        : m_left_size(left_size),
          m_right_size(right_size),
          m_graph(left_size) {}

      const std::vector<std::vector<int>>& get_graph() const {
        return m_graph;
      }

      void add_edge(int u, int v) {
        m_graph[u].push_back(v);
      }

      int get_max_matching() {
        m_pair_u.assign(m_left_size, -1);
        m_pair_v.assign(m_right_size, -1);
        auto matching = 0;
        while(bfs()) {
          for(int u = 0; u < m_left_size; ++u) {
            if(m_pair_u[u] == -1 && dfs(u)) {
              ++matching;
            }
          }
        }
        return matching;
      }

    private:
      int m_left_size;
      int m_right_size;
      std::vector<std::vector<int>> m_graph;
      std::vector<int> m_pair_u;
      std::vector<int> m_pair_v;
      std::vector<int> m_dist;

      bool bfs() {
        m_dist.assign(m_left_size, std::numeric_limits<int>::max());
        auto q = std::queue<int>();
        for(auto u = 0; u < m_left_size; ++u) {
          if(m_pair_u[u] == -1) {
            m_dist[u] = 0;
            q.push(u);
          }
        }
        auto found = false;
        while(!q.empty()) {
          auto u = q.front();
          q.pop();
          for(auto v : m_graph[u]) {
            if(m_pair_v[v] == -1) {
              found = true;
            } else if(m_dist[m_pair_v[v]] == std::numeric_limits<int>::max()) {
              m_dist[m_pair_v[v]] = m_dist[u] + 1;
              q.push(m_pair_v[v]);
            }
          }
        }
        return found;
      }

      bool dfs(int u) {
        for(auto v : m_graph[u]) {
          if(m_pair_v[v] == -1 ||
              (m_dist[m_pair_v[v]] == m_dist[u] + 1 && dfs(m_pair_v[v]))) {
            m_pair_u[u] = v;
            m_pair_v[v] = u;
            return true;
          }
        }
        m_dist[u] = std::numeric_limits<int>::max();
        return false;
      }
  };
}

bool Spire::matches(const QString& query, const std::vector<QString>& words) {
  auto query_words =
    query.toLower().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  auto query_size = query_words.size();
  auto word_size = words.size();
  if(query_size > word_size) {
    return true;
  }
  auto hopcroft_karp = HopcroftKarp(query_size, word_size);
  for(auto i = 0; i < query_size; ++i) {
    for(auto j = 0; j < word_size; ++j) {
      if(words[j].startsWith(query_words[i], Qt::CaseInsensitive)) {
        hopcroft_karp.add_edge(i, j);
      }
    }
    if(hopcroft_karp.get_graph()[i].empty()) {
      return false;
    }
  }
  return hopcroft_karp.get_max_matching() == query_size;
}
