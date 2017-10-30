import ChainableModel from './chainable-model';
import DataChangeType from './data-change-type';
import definitionsService from 'services/definitions';

export default class extends ChainableModel{
  constructor(sourceModel, columnSortOrders, valueComparer) {
    super(sourceModel);
    this.valueComparer = valueComparer;
    this.onDataChange = this.onDataChange.bind(this);
    this.dataChangeSubId = this.sourceModel.addDataChangeListener(this.onDataChange);

    this.columnSortOrders = columnSortOrders;
    this.sourceToSortedIndices = [];
    this.sortedToSourceIndices = [];

    // initialize sorting with given source model for already existing data
    this.initialize();
  }

  /** @private */
  initialize() {
    let rowCount = this.sourceModel.getRowCount();
    for (let i=0; i<rowCount; i++) {
      this.addData(i);
    }
  }

  getColumnSortOrders() {
    return this.columnSortOrders;
  }

  getRowCount() {
    return this.sourceModel.getRowCount();
  }

  getColumnCount() {
    return this.sourceModel.getColumnCount();
  }

  getColumnHeader(x) {
    return this.sourceModel.getColumnHeader(x);
  }

  getValueAt(x, y) {
    let sourceIndex = this.sortedToSourceIndices[y];
    return this.sourceModel.getValueAt(x, sourceIndex);
  }

  onDataChange(dataChangeType, rowIndex, toIndex) {
    if (dataChangeType == DataChangeType.ADD) {
      this.handleDataAdd(rowIndex);
    } else if (dataChangeType == DataChangeType.UPDATE) {
      this.handleDataUpdate(rowIndex);
    } else if (dataChangeType == DataChangeType.REMOVE) {
      this.handleDataRemove(rowIndex);
    } else {

    }
  }

  /** @private */
  handleDataAdd(rowIndex) {
    let sortedIndex = this.addData(rowIndex);
    let listeners = this.dataChangeListeners.values();
    for (let i=0; i<listeners.length; i++) {
      listeners[i](DataChangeType.ADD, sortedIndex);
    }
  }

  /** @private */
  addData(rowIndex) {
    let sortedIndex;
    if (this.columnSortOrders.length == 0) {
      sortedIndex = rowIndex;
    } else {
      let values = this.getValuesOfSortOrderColumns(rowIndex);
      sortedIndex = this.searchIndex(values);
    }

    for (let i=0; i<this.sourceToSortedIndices.length; i++) {
      if (this.sourceToSortedIndices[i] >= sortedIndex) {
        this.sourceToSortedIndices[i]++;
      }
    }

    this.sortedToSourceIndices.splice(sortedIndex, 0, rowIndex);
    this.sourceToSortedIndices.splice(rowIndex, 0, sortedIndex);

    return sortedIndex;
  }

  /** @private */
  handleDataUpdate(rowIndex) {
    let prevSortedIndex = this.sourceToSortedIndices[rowIndex];
    this.sortedToSourceIndices.splice(prevSortedIndex, 1);

    let sortedIndex;
    if (this.columnSortOrders.length == 0) {
      sortedIndex = rowIndex;
    } else {
      let updatedValues = this.getValuesOfSortOrderColumns(rowIndex);
      sortedIndex = this.searchIndex(updatedValues);
    }

    this.sortedToSourceIndices.splice(sortedIndex, 0, rowIndex);
    for (let i=0; i<this.sortedToSourceIndices.length; i++) {
      let sourceIndex = this.sortedToSourceIndices[i];
      this.sourceToSortedIndices[sourceIndex] = i;
    }

    // notify the listeners of the changes
    let listeners = this.dataChangeListeners.values();
    if (prevSortedIndex != sortedIndex) {
      // notify MOVE
      for (let i=0; i<listeners.length; i++) {
        listeners[i](DataChangeType.MOVE, prevSortedIndex, sortedIndex);
      }
    }

    // notify UPDATE
    for (let i=0; i<listeners.length; i++) {
      listeners[i](DataChangeType.UPDATE, sortedIndex);
    }
  }

  /** @private */
  handleDataRemove(rowIndex) {
    let sortedIndex = this.sourceToSortedIndices[rowIndex];
    this.sourceToSortedIndices.splice(rowIndex, 1);
    this.sortedToSourceIndices.splice(sortedIndex, 1);

    for (let i=0; i<this.sourceToSortedIndices.length; i++) {
      if (this.sourceToSortedIndices[i] > sortedIndex) {
        this.sourceToSortedIndices[i]--;
      }
    }

    for (let i=0; i<this.sortedToSourceIndices.length; i++) {
      if (this.sortedToSourceIndices[i] > rowIndex) {
        this.sortedToSourceIndices[i]--;
      }
    }
  }

  /** @private */
  searchIndex(values) {
    // binary search
    let startIndex = 0;
    let endIndex = this.sortedToSourceIndices.length - 1;
    while (startIndex + 2 <= endIndex) {
      // loop until we have the smallest possible array in our search
      let midIndex = Math.ceil((startIndex + endIndex) / 2);
      let sourceIndex = this.sortedToSourceIndices[midIndex];
      let midValues = this.getValuesOfSortOrderColumns(sourceIndex);
      let compareResult = this.compareValuesBySortOrders(values, midValues);
      if (compareResult == 0) {
        return this.findHeadOfDuplicates(midIndex);
      } else if (compareResult < 0) {
        endIndex = midIndex - 1;
      } else if (compareResult > 0) {
        startIndex = midIndex + 1;
      }
    }

    if (endIndex == -1) {
      // very beginning without any values
      return startIndex;
    } else {
      // compare value at start index
      let startSourceIndex = this.sortedToSourceIndices[startIndex];
      let startValues = this.getValuesOfSortOrderColumns(startSourceIndex);
      let compareToStartResult = this.compareValuesBySortOrders(values, startValues);
      if (compareToStartResult <= 0) {
        return startIndex;
      }

      // compare value at end index
      let endSourceIndex = this.sortedToSourceIndices[endIndex];
      let endValues = this.getValuesOfSortOrderColumns(endSourceIndex);
      let compareToEndResult = this.compareValuesBySortOrders(values, endValues);
      if (compareToEndResult <= 0) {
        return endIndex;
      }

      if (compareToEndResult > 0) {
        return endIndex + 1;
      }
    }
  }

  /** @private */
  findHeadOfDuplicates(sortedIndex) {
    if (sortedIndex == 0) {
      return sortedIndex;
    }

    let isDuplicate = true;
    while (isDuplicate) {
      let sourceIndex = this.sortedToSourceIndices[sortedIndex];
      let currentValues = this.getValuesOfSortOrderColumns(sourceIndex);
      let sourceIndexOneBefore = this.sortedToSourceIndices[sortedIndex - 1];
      if (sourceIndexOneBefore == null) {
        break;
      }
      let valuesOneBefore = this.getValuesOfSortOrderColumns(sourceIndexOneBefore);
      let compareResult = this.compareValuesBySortOrders(currentValues, valuesOneBefore);
      if (compareResult == 0) {
        sortedIndex--;
      } else {
        isDuplicate = false;
      }
    }

    return sortedIndex;
  }

  /** @private */
  getValuesOfSortOrderColumns(rowIndex) {
    let values = [];
    for (let i=0; i<this.columnSortOrders.length; i++) {
      let value = this.sourceModel.getValueAt(this.columnSortOrders[i].index, rowIndex);
      values.push(value);
    }
    return values;
  }

  /** @private */
  compareValuesBySortOrders(aValues, bValues) {
    for (let i=0; i<this.columnSortOrders.length; i++) {
      let isAsc = this.columnSortOrders[i].isAsc;
      let compareResult = this.compareValues(aValues[i], bValues[i], isAsc);
      if (compareResult != 0) {
        return compareResult;
      }
    }
    return 0;
  }

  /** @private */
  compareValues(a, b, isAsc) {
    let result = this.valueComparer.compare(a, b);
    if (isAsc) {
      return result;
    } else {
      return -1 * result;
    }
  }
}
