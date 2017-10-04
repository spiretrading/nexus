import DataChangeType from './data-change-type';
import uuid from 'uuid';
import HashMap from 'hashmap';
import ValueComparer from './value-comparer';

export default class {
  constructor(sourceModel) {
    this.sourceModel = sourceModel;
    this.onDataChange = this.onDataChange.bind(this);
    this.dataChangeSubId = this.sourceModel.addDataChangeListener(this.onDataChange);
    this.dataChangeListeners = new HashMap();
    this.lastValuesCache = [];  // 2D array of row x column
    this.totals = [];
  }

  getRowCount() {
    return this.sourceModel.getRowCount();
  }

  getColumnCount() {
    return this.sourceModel.getColumnCount();
  }

  getValueAt(x, y) {
    return this.totals[x];
  }

  onDataChange(dataChangeType, rowIndex, toIndex) {
    if (dataChangeType == DataChangeType.ADD) {
      this.handleDataAdd(rowIndex);
    } else if (dataChangeType == DataChangeType.UPDATE) {
      this.handleDataUpdate(rowIndex);
    } else if (dataChangeType == DataChangeType.REMOVE) {
      this.handleDataRemove(rowIndex);
    }
  }

  /** @private */
  handleDataAdd(rowIndex) {
    // cache the new row
    let row = [];
    let columnCount = this.sourceModel.getColumnCount();
    for (let i=0; i<columnCount; i++) {
      let value = this.sourceModel.getValueAt(i, rowIndex);
      row.push(value);
      this.addTotal(i, value);
    }
    this.lastValuesCache.push(row);

    // notify the listeners
    let listeners = this.dataChangeListeners.values();
    for (let i=0; i<listeners.length; i++) {
      listeners[i](DataChangeType.UPDATE, 0);
    }
  }

  /** @private */
  addTotal(columnIndex, value) {
    if (this.totals[columnIndex] == null) {
      this.totals.push(value);
    } else {
      let constructorName = value.constructor.name;
      if (constructorName == 'Money') {
        this.totals[columnIndex] = this.totals[columnIndex].add(value);
      } else {
        this.totals[columnIndex] = this.totals[columnIndex] + value;
      }
    }
  }

  /** @private */
  handleDataUpdate(rowIndex) {
    let columnCount = this.sourceModel.getColumnCount();
    for (let i=0; i<columnCount; i++) {
      let newValue = this.sourceModel.getValueAt(i, rowIndex);
      let lastValue = this.lastValuesCache[rowIndex][i];
      let constructorName = lastValue.constructor.name;
      let delta;
      if (constructorName == 'Money') {
        delta = newValue.subtract(lastValue);
        this.totals[i] = this.totals[i].add(delta);
      } else {
        delta = newValue - lastValue;
        this.totals[i] = this.totals[i] + delta;
      }
      this.lastValuesCache[rowIndex][i] = newValue;
    }

    // notify the listeners
    let listeners = this.dataChangeListeners.values();
    for (let i=0; i<listeners.length; i++) {
      listeners[i](DataChangeType.UPDATE, 0);
    }
  }

  /** @private */
  handleDataRemove(rowIndex) {
    let removedRow = this.lastValuesCache[rowIndex];
    this.lastValuesCache.splice(rowIndex, 1);
    let columnCount = this.sourceModel.getColumnCount();

    for (let i=0; i<columnCount; i++) {
      let constructorName = this.totals[i].constructor.name;
      if (constructorName == 'Money') {
        this.totals[i] = this.totals[i].subtract(removedRow[i]);
      } else {
        this.totals[i] = this.totals[i] - removedRow[i];
      }
    }

    // notify the listeners
    let listeners = this.dataChangeListeners.values();
    for (let i=0; i<listeners.length; i++) {
      listeners[i](DataChangeType.UPDATE, 0);
    }
  }

  addDataChangeListener(listener) {
    let subId = uuid.v4();
    this.dataChangeListeners.set(subId, listener);
    return subId;
  }

  removeDataChangeListener(subId) {
    this.dataChangeListeners.remove(subId);
  }
}
