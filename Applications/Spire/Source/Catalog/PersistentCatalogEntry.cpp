#include "Spire/Catalog/PersistentCatalogEntry.hpp"

using namespace boost;
using namespace boost::uuids;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

PersistentCatalogEntry::~PersistentCatalogEntry() {}

PersistentCatalogEntry::PersistentCatalogEntry(const uuid& uid)
    : CatalogEntry(uid) {}

PersistentCatalogEntry::PersistentCatalogEntry() {}
