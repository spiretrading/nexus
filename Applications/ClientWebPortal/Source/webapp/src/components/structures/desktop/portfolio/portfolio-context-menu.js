import ContextMenu from 'components/reusables/common/big-table/context-menu';
import HashMap from 'hashmap';

export default class extends ContextMenu {
  constructor(menuItems) {
    super(menuItems);
  }

  setTableModel(tableModel) {
    this.tableModel = tableModel;
  }

  onMenuItemClick(item) {
    if (this.tableModel == null) {
      throw new TypeError('Table model must be set.');
    }

    if (item == 'viewProfile') {
      let selectedRows = this.tableModel.getSelectedRows();
      let selectedIds = new HashMap();
      for (let i=0; i<selectedRows.length; i++) {
        let directoryEntry = this.tableModel.getValueAt(0, selectedRows[i]).value;
        selectedIds.set(directoryEntry.id, directoryEntry);
      }
      let directoryEntries = selectedIds.values();
      for (let i=0; i<directoryEntries.length; i++) {
        let directoryEntry = directoryEntries[i];
        let url = window.location.origin + '/profile-account/' +
          directoryEntry.type + '/' + directoryEntry.id + '/' + directoryEntry.name;
        window.open(url, '_blank');
      }
    }
  }
}
