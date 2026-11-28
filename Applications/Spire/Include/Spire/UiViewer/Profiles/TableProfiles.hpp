#ifndef SPIRE_UI_VIEWER_TABLE_PROFILES_HPP
#define SPIRE_UI_VIEWER_TABLE_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the EditableTableView. */
  UiProfile make_editable_table_view_profile();

  /** Returns a UiProfile for the grouped TableView. */
  UiProfile make_grouped_table_view_profile();

  /** Returns a UiProfile for the TableHeader. */
  UiProfile make_table_header_profile();

  /** Returns a UiProfile for the TableHeaderItem. */
  UiProfile make_table_header_item_profile();

  /** Returns a UiProfile for the TableView. */
  UiProfile make_table_view_profile();
}

#endif
