#ifndef NEXUS_DATABASE_FACET_HPP
#define NEXUS_DATABASE_FACET_HPP
#include <concepts>
#include <locale>
#include <utility>

namespace Nexus {

  /**
   * Supplies an owned database for locale-based formatting.
   * @tparam D The type of database.
   */
  template<typename D>
  class DatabaseFacet : public std::locale::facet {
    public:

      /** The type of database. */
      using Database = D;

      inline static std::locale::id id;

      /**
       * Constructs a facet owning the given database.
       * @param database The database to supply for formatting.
       */
      explicit DatabaseFacet(Database database);

      /** Returns the database supplied by this facet. */
      const Database& get_database() const;

    private:
      Database m_database;
  };

  /**
   * Returns a locale with the given databases added or replaced by type.
   * @param locale The base locale.
   * @param databases The databases to supply for formatting.
   */
  std::locale make_locale(std::locale locale, auto... databases) {
    ((locale =
      std::locale(locale, new DatabaseFacet(std::move(databases)))), ...);
    return locale;
  }

  /**
   * Returns a classic locale with the given databases added by type.
   * @param database The first database to supply for formatting.
   * @param databases Additional databases, replacing earlier ones of the same
   *        type.
   */
  std::locale make_locale(auto database, auto... databases) requires
      (!std::same_as<decltype(database), std::locale>) {
    return make_locale(
      std::locale::classic(), std::move(database), std::move(databases)...);
  }

  template<typename D>
  DatabaseFacet<D>::DatabaseFacet(Database database)
    : m_database(std::move(database)) {}

  template<typename D>
  const typename DatabaseFacet<D>::Database&
      DatabaseFacet<D>::get_database() const {
    return m_database;
  }
}

#endif
