import DataChangeType from 'utils/table-models/model/data-change-type';
import Model from 'utils/table-models/model';
import SignalManager from 'utils/signal-manager';

export default class extends Model {
  constructor(columnNames) {
    super();
    if (columnNames == null) {
      throw new TypeError('Column names cannot be null');
    }

    this.data = [];
    this.columnNames = columnNames.slice();
    this.signalManager = new SignalManager();
  }

  getRowCount() {
    return this.data.length;
  }

  getColumnCount() {
    return this.columnNames.length;
  }

  getColumnName(columnIndex) {
    if (columnIndex >= this.columnNames.length || columnIndex < 0) {
      let errorMessage = 'Invalid column index. Max: ' + (this.columnNames.length - 1) + ', Actual: ' + columnIndex;
      throw new RangeError(errorMessage);
    }
    return this.columnNames[columnIndex];
  }

  getValueAt(x, y) {
    if (y >= this.data.length || y < 0) {
      let errorMessage = 'Invalid row index. Max: ' + (this.data.length - 1) + ', Actual: ' + y;
      throw new RangeError(errorMessage);
    } else if (x >= this.columnNames.length || x < 0) {
      let errorMessage = 'Invalid column index. Max: ' + (this.columnNames.length - 1) + ', Actual: ' + x;
      throw new RangeError(errorMessage);
    }
    return this.data[y][x];
  }

  addRow(rowData) {
    if (rowData.length != this.columnNames.length) {
      let errorMessage = 'Mismatching data length. Expecting: ' + this.columnNames.length + ', Actual: ' + rowData.length;
      throw new RangeError(errorMessage);
    }
    this.data.push(Object.freeze(rowData.slice()));
    let rowIndex = this.data.length - 1;
    this.signalManager.emitSignal(DataChangeType.ADD, rowIndex);
  }

  removeRow(rowIndex) {
    if (rowIndex >= this.data.length || rowIndex < 0) {
      let errorMessage = 'Invalid row index. Max: ' + (this.data.length - 1) + ', Actual: ' + rowIndex;
      throw new RangeError(errorMessage);
    }
    let removedRows = this.data.splice(rowIndex, 1);
    this.signalManager.emitSignal(DataChangeType.REMOVE, {
      index: rowIndex,
      row: removedRows[0]
    });
  }

  updateRow(rowIndex, rowData) {
    if (rowIndex >= this.data.length || rowIndex < 0) {
      let errorMessage = 'Invalid row index. Max: ' + (this.data.length - 1) + ', Actual: ' + rowIndex;
      throw new RangeError(errorMessage);
    }
    let original = this.data[rowIndex];
    this.data[rowIndex] = rowData.slice();
    this.signalManager.emitSignal(DataChangeType.UPDATE, {
      index: rowIndex,
      original: original
    });
  }

  addDataChangeListener(listener) {
    return this.signalManager.addListener(listener);
  }

  removeDataChangeListener(subId) {
    this.signalManager.removeListener(subId);
  }
}
