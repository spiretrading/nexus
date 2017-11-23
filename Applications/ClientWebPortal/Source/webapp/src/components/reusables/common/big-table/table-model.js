import numberFormatter from 'utils/number-formatter';
import HashMap from 'hashmap';

class TableModel {
  constructor(dataModel) {
    this.dataModel = dataModel;
    this.rowCount = 0;
    this.selectedRowsControlModApplied = new HashMap();
  }

  getRowCount() {
    return this.rowCount;
  }

  getColumnCount() {
    return this.dataModel.getColumnCount();
  }

  getColumnHeader(x) {
    return this.dataModel.getColumnName(x);
  }

  getValueAt(x, y) {
    return this.dataModel.getValueAt(x, y);
  }

  updateColumnChange(dataModel) {
    // reset variables
    this.dataModel = dataModel;
  }

  rowUpdate(rowIndex) {

  }

  rowAdd(rowIndex) {
    this.rowCount++;

    // update selected rows
    for (let i=this.rowCount-1; i>=rowIndex; i--) {
      if (this.selectedRowsControlModApplied.has(i)) {
        let previousValue = this.selectedRowsControlModApplied.get(i);
        this.selectedRowsControlModApplied.set(i+1, previousValue);
        this.selectedRowsControlModApplied.remove(i);
      }
    }
  }

  rowRemove(rowIndex) {
    this.rowCount--;

    // update selected rows
    for (let i=rowIndex; i<this.rowCount; i++) {
      if (this.selectedRowsControlModApplied.has(i)) {
        let previousValue = this.selectedRowsControlModApplied.get(i);
        this.selectedRowsControlModApplied.set(i-1, previousValue);
        this.selectedRowsControlModApplied.remove(i);
      }
    }
  }

  rowMove(fromIndex, toIndex) {
    let prevValue;
    // let nextValue = this.allLengthsCache[fromIndex];
    let nextSelectedRowValue = this.selectedRowsControlModApplied.get(fromIndex);
    this.selectedRowsControlModApplied.remove(fromIndex);
    if (fromIndex < toIndex) {
      for (let i=toIndex; i>=fromIndex; i--) {
        // update the selected rows
        let valueToUpdateTo = nextSelectedRowValue;
        nextSelectedRowValue = this.selectedRowsControlModApplied.get(i);
        this.selectedRowsControlModApplied.remove(i);
        if (valueToUpdateTo != null) {
          this.selectedRowsControlModApplied.set(i, valueToUpdateTo);
        }
      }
    } else {
      for (let i=toIndex; i<=fromIndex; i++) {
        // update the selected rows
        let valueToUpdateTo = nextSelectedRowValue;
        nextSelectedRowValue = this.selectedRowsControlModApplied.get(i);
        this.selectedRowsControlModApplied.remove(i);
        if (valueToUpdateTo != null) {
          this.selectedRowsControlModApplied.set(i, valueToUpdateTo);
        }
      }
    }
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

export default TableModel;
