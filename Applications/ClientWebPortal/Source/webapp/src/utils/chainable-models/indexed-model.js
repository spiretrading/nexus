import ArrayModel from './array-model';
import HashMap from 'hashmap';
import ArrayStringKeyGenerator from 'utils/array-string-key-generator';

export default class {
  constructor(indices, columnNames, keyGenerator = new ArrayStringKeyGenerator()) {
    // validate indices
    for (let i=0; i<indices.length; i++) {
      if (indices[i] < 0 || indices[i] >= columnNames.length) {
        let errorMessage = 'Index ' + indices[i] + ' is out of bounds';
        throw new RangeError(errorMessage);
      }
    }

    this.indices = indices.slice(0);
    this.arrayModel = new ArrayModel(columnNames);
    this.keyGenerator = keyGenerator;
    this.indicesToRow = new HashMap();
  }

  getRowCount() {
    return this.arrayModel.getRowCount();
  }

  getColumnCount() {
    return this.arrayModel.getColumnCount();
  }

  getColumnName(columnIndex) {
    return this.arrayModel.getColumnName(columnIndex);
  }

  getValueAt(x, y) {
    return this.arrayModel.getValueAt(x, y);
  }

  update(values) {
    let indexValues = [];
    for (let i=0; i<this.indices.length; i++) {
      indexValues.push(values[i]);
    }
    let key = this.keyGenerator.get(indexValues);

    if (!this.indicesToRow.has(key)) {
      let rowNumber = this.arrayModel.getRowCount();
      this.indicesToRow.set(key, rowNumber);
      this.arrayModel.addRow(values);
    } else {
      let rowNumber = this.indicesToRow.get(key);
      this.arrayModel.updateRow(rowNumber, values);
    }
  }

  removeRow(values) {
    let indexValues = [];
    for (let i=0; i<this.indices.length; i++) {
      indexValues.push(values[i]);
    }
    let key = this.keyGenerator.get(indexValues);
    if (this.indicesToRow.has(key)) {
      let rowNumber = this.indicesToRow.get(key);
      this.arrayModel.removeRow(rowNumber);
    }
  }

  addDataChangeListener(listener) {
    return this.arrayModel.addDataChangeListener(listener);
  }

  removeDataChangeListener(subId) {
    this.arrayModel.removeDataChangeListener(subId);
  }
}
