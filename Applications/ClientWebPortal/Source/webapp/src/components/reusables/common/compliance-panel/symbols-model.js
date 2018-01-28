import HashMap from 'hashmap';

export default class {
  constructor() {
    this.selectedRowsControlModApplied = new HashMap();
  }

  isRowSelected(rowIndex) {
    return this.selectedRowsControlModApplied.has(rowIndex);
  }

  isRowSelectedControlModified(rowIndex) {
    return this.selectedRowsControlModApplied.has(rowIndex) && this.selectedRowsControlModApplied.get(rowIndex);
  }

  clearSelectedRows() {
    this.selectedRowsControlModApplied.clear();
  }

  setSelectedRow(rowIndex, isSelected) {
    this.selectedRowsControlModApplied.set(rowIndex, isSelected);
  }

  removeRowSelection(rowIndex) {
    this.selectedRowsControlModApplied.remove(rowIndex);
  }

  setAllSelectedRowsControlModified() {
    this.selectedRowsControlModApplied.forEach((value, key) => {
      this.selectedRowsControlModApplied.set(key, true);
    });
  }

  removeNonControlModifiedSelectedRows() {
    let selectedRowsClone = this.selectedRowsControlModApplied.clone();
    selectedRowsClone.forEach(function(value, key) {
      if (!value) {
        this.selectedRowsControlModApplied.remove(key);
      }
    }.bind(this));
  }

  getSelectedRows() {
    return this.selectedRowsControlModApplied.keys();
  }
}
