import numberFormatter from 'utils/number-formatter';
import currencyFormatter from 'utils/currency-formatter';
import HashMap from 'hashmap';

class TableModel {
  constructor(dataModel) {
    this.dataModel = dataModel;
    this.allLengthsCache = [];
    this.columnMaxLengthCells = [];
    this.selectedRowsControlModApplied = new HashMap();
  }

  getColumnLengths() {
    return this.columnMaxLengthCells;
  }

  /** @private */
  updateColumnMaxLength(columnIndex) {
    let max = 0;
    let maxRowIndex;
    for (let rowIndex=0; rowIndex<this.getRowCount(); rowIndex++) {
      let cellLength = this.allLengthsCache[rowIndex][columnIndex];
      if (max < cellLength) {
        max = cellLength;
        maxRowIndex = rowIndex;
      }
    }
    this.columnMaxLengthCells[columnIndex] = {
      rowIndex: maxRowIndex,
      length: max
    };
  }

  getRowCount() {
    return this.allLengthsCache.length;
  }

  getColumnCount() {
    return this.dataModel.getColumnCount();
  }

  getColumnHeader(x) {
    return this.dataModel.getColumnHeader(x);
  }

  getValueAt(x, y) {
    return this.dataModel.getValueAt(x, y);
  }

  updateColumnChange(dataModel) {
    // reset variables
    this.dataModel = dataModel;
    this.allLengthsCache = [];
    this.columnMaxLengthCells = [];

    // initialize empty caches
    for (let i=0; i<this.getRowCount(); i++) {
      this.rowAdd(i);
    }
  }

  rowUpdate(rowIndex) {
    // update cell sizes cache
    let columnCount = this.getColumnCount();
    let cellLengths = [];
    for (let columnIndex=0; columnIndex<columnCount; columnIndex++) {
      let cellValue = this.getValueAt(columnIndex, rowIndex);
      cellLengths.push(cellValue.length);
      // update column max length cells
      if (cellValue.length > this.columnMaxLengthCells[columnIndex].length) {
        this.columnMaxLengthCells[columnIndex] = {
          rowIndex: rowIndex,
          length: cellValue.length
        };
      } else if (rowIndex == this.columnMaxLengthCells[columnIndex].rowIndex && cellValue.length < this.columnMaxLengthCells[columnIndex].length) {
        this.updateColumnMaxLength(columnIndex);
      }
    }
    this.allLengthsCache[rowIndex] = cellLengths;
  }

  rowAdd(rowIndex) {
    // update cell sizes cache
    let columnCount = this.getColumnCount();
    let cellLengths = [];
    for (let columnIndex=0; columnIndex<columnCount; columnIndex++) {
      let cellValue = this.getValueAt(columnIndex, rowIndex);
      cellLengths.push(cellValue.length);
      // update column max length cells
      if (this.columnMaxLengthCells[columnIndex] == null) {
        this.columnMaxLengthCells.push({
          rowIndex: rowIndex,
          length: cellValue.length
        });
      } else if (cellValue.length > this.columnMaxLengthCells[columnIndex].length) {
        this.columnMaxLengthCells[columnIndex] = {
          rowIndex: rowIndex,
          length: cellValue.length
        };
      }
    }
    this.allLengthsCache.splice(rowIndex, 0, cellLengths);

    // update selected rows
    for (let i=this.allLengthsCache.length-1; i>=rowIndex; i--) {
      if (this.selectedRowsControlModApplied.has(i)) {
        let previousValue = this.selectedRowsControlModApplied.get(i);
        this.selectedRowsControlModApplied.set(i+1, previousValue);
        this.selectedRowsControlModApplied.remove(i);
      }
    }
  }

  rowRemove(rowIndex) {
    // update selected rows
    for (let i=rowIndex; i<this.allLengthsCache.length; i++) {
      if (this.selectedRowsControlModApplied.has(i)) {
        let previousValue = this.selectedRowsControlModApplied.get(i);
        this.selectedRowsControlModApplied.set(i-1, previousValue);
        this.selectedRowsControlModApplied.remove(i);
      }
    }

    // update cell size cache
    this.allLengthsCache.splice(rowIndex, 1);
    for (let columnIndex=0; columnIndex<this.columnMaxLengthCells.length; columnIndex++) {
      let maxRow = this.columnMaxLengthCells[columnIndex].rowIndex;
      if (maxRow == rowIndex) {
        this.updateColumnMaxLength(columnIndex);
      }
    }
  }

  rowMove(fromIndex, toIndex) {
    let prevValue;
    let nextValue = this.allLengthsCache[fromIndex];
    let nextSelectedRowValue = this.selectedRowsControlModApplied.get(fromIndex);
    this.selectedRowsControlModApplied.remove(fromIndex);
    if (fromIndex < toIndex) {
      for (let i=toIndex; i>=fromIndex; i--) {
        // update the length cache
        prevValue = this.allLengthsCache[i];
        this.allLengthsCache[i] = nextValue;
        nextValue = prevValue;

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
        // update the length cache
        prevValue = this.allLengthsCache[i];
        this.allLengthsCache[i] = nextValue;
        nextValue = prevValue;

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

  isRowSelectedControlModified(rowIndex) {
    return this.selectedRowsControlModApplied.has(rowIndex);
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
}

export default TableModel;
