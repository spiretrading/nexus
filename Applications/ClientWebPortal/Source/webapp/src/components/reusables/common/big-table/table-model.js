class TableModel {
  constructor(dataModel) {
    this.dataModel = dataModel;
    this.allLengthsCache = [];
    this.columnMaxLengthCells = [];
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
    return this.dataModel.getRowCount();
  }

  getColumnCount() {
    return this.dataModel.getColumnCount();
  }

  getValueAt(x, y) {
    let value = this.dataModel.getValueAt(x, y);
    return this.formatValue(value, x);
  }

  rowUpdate(rowIndex) {
    // update cell sizes cache
    let columnCount = this.getColumnCount();
    let cellLengths = [];
    for (let columnIndex=0; columnIndex<columnCount; columnIndex++) {
      let cellValue = this.getValueAt(columnIndex, rowIndex);
      cellValue = this.formatValue(cellValue, columnIndex);
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
      cellValue = this.formatValue(cellValue, columnIndex);
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
  }

  rowRemove(rowIndex) {
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
    if (fromIndex < toIndex) {
      for (let i=toIndex; i>=fromIndex; i--) {
        prevValue = this.allLengthsCache[i];
        this.allLengthsCache[i] = nextValue;
        nextValue = prevValue;
      }
    } else {
      for (let i=toIndex; i>=fromIndex; i++) {
        prevValue = this.allLengthsCache[i];
        this.allLengthsCache[i] = nextValue;
        nextValue = prevValue;
      }
    }
  }

  /** @private */
  formatValue(value, x) {
    return value;
  }
}

export default TableModel;
