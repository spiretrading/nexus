import DataChangeType from './data-change-type';
import uuid from 'uuid';
import HashMap from 'hashmap';
import definitionsService from 'services/definitions';

export default class {
  constructor(sourceModel) {
    this.sourceModel = sourceModel;
    this.onDataChange = this.onDataChange.bind(this);
    this.dataChangeSubId = this.sourceModel.addDataChangeListener(this.onDataChange);
    this.dataChangeListeners = new HashMap();

    this.sortingColumnIndex = 0;
    this.sourceToSortedIndices = [];
    this.sortedToSourceIndices = [];
    this.sortedColumnValues = [];
  }

  getRowCount() {
    return this.sourceModel.getRowCount();
  }

  getColumnCount() {
    return this.sourceModel.getColumnCount();
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
    let value = this.sourceModel.getValueAt(this.sortingColumnIndex, rowIndex);
    let sortedIndex = this.searchIndex(value);

    for (let i=0; i<this.sourceToSortedIndices.length; i++) {
      if (this.sourceToSortedIndices[i] >= sortedIndex) {
        this.sourceToSortedIndices[i]++;
      }
    }

    for (let i=0; i<this.sortedToSourceIndices.length; i++) {
      if (this.sortedToSourceIndices[i] >= rowIndex) {
        this.sortedToSourceIndices[i]++;
      }
    }

    this.sortedColumnValues.splice(sortedIndex, 0, value);
    this.sortedToSourceIndices.splice(sortedIndex, 0, rowIndex);
    this.sourceToSortedIndices.splice(rowIndex, 0, sortedIndex);

    let listeners = this.dataChangeListeners.values();
    for (let i=0; i<listeners.length; i++) {
      listeners[i](DataChangeType.ADD, sortedIndex);
    }
  }

  /** @private */
  handleDataUpdate(rowIndex) {
    let listeners = this.dataChangeListeners.values();
    let value = this.sourceModel.getValueAt(this.sortingColumnIndex, rowIndex);
    let oldIndex = this.sourceToSortedIndices[rowIndex];
    let newIndex = this.searchIndex(value);
    if (newIndex == oldIndex) {
      // sort order hasn't changed
      this.sortedColumnValues[oldIndex] = value;
      for (let i=0; i<listeners.length; i++) {
        listeners[i](DataChangeType.UPDATE, oldIndex);
      }
    } else {
      // sort order has changed
      // iterate and update values between the old and new
      let prevValue, prevIndex;
      let nextValue = value;
      let nextIndex = this.sortedToSourceIndices[oldIndex];
      if (newIndex > oldIndex) {
        newIndex--;   // deduct 1 value due to the nature of searchIndex implementation
        for (let i=newIndex; i>=oldIndex; i--) {
          // update source index mapping
          let sourceIndex = this.sortedToSourceIndices[i];
          this.sourceToSortedIndices[sourceIndex]--;
          // move the values and index
          prevValue = this.sortedColumnValues[i];
          prevIndex = this.sortedToSourceIndices[i];
          this.sortedColumnValues[i] = nextValue;
          this.sortedToSourceIndices[i] = nextIndex;
          nextValue = prevValue;
          nextIndex = prevIndex;
        }
      } else if (newIndex < oldIndex) {
        for (let i=newIndex; i<=oldIndex; i++) {
          // update source index mapping
          let sourceIndex = this.sortedToSourceIndices[i];
          this.sourceToSortedIndices[sourceIndex]++;
          // move the values and index
          prevValue = this.sortedColumnValues[i];
          prevIndex = this.sortedToSourceIndices[i];
          this.sortedColumnValues[i] = nextValue;
          this.sortedToSourceIndices[i] = nextIndex;
          nextValue = prevValue;
          nextIndex = prevIndex;
        }
      }

      // notify MOVE
      for (let i=0; i<listeners.length; i++) {
        listeners[i](DataChangeType.MOVE, oldIndex, newIndex);
      }

      // notify UPDATE
      if (oldIndex < newIndex) {
        for (let i=oldIndex; i<=newIndex; i++) {
          for (let i=0; i<listeners.length; i++) {
            listeners[i](DataChangeType.UPDATE, i);
          }
        }
      } else {
        for (let i=newIndex; i<=oldIndex; i++) {
          for (let i=0; i<listeners.length; i++) {
            listeners[i](DataChangeType.UPDATE, i);
          }
        }
      }
    }
  }

  /** @private */
  handleDataRemove(rowIndex) {
    let sortedIndex = this.sourceToSortedIndices[rowIndex];
    this.sourceToSortedIndices.splice(rowIndex, 1);
    this.sortedColumnValues.splice(sortedIndex, 1);
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
  searchIndex(value) {
    // binary search
    let startIndex = 0;
    let endIndex = this.sortedColumnValues.length - 1;
    while (startIndex + 2 <= endIndex) {
      // loop until we have the smallest possible array in our search
      let midIndex = Math.ceil((startIndex + endIndex) / 2);
      let midValue = this.sortedColumnValues[midIndex];
      let compareResult = this.compareValues(value, midValue);
      if (compareResult == 0) {
        return midIndex;
      } else if (compareResult < 0) {
        endIndex = midIndex - 1;
      } else if (compareResult > 0) {
        startIndex = midIndex + 1;
      }
    }

    let startValue = this.sortedColumnValues[startIndex];
    let endValue = this.sortedColumnValues[endIndex];
    if (endIndex == -1) {
      // very beginning without any values
      return startIndex;
    } else {
      let compareToStartResult = this.compareValues(value, startValue);
      let compareToEndResult = this.compareValues(value, endValue);
      if (compareToStartResult <= 0) {
        return startIndex;
      } else if (compareToEndResult <= 0) {
        return endIndex;
      } else {
        return endIndex + 1;
      }
    }
  }

  /** @private */
  compareValues(a, b) {
    let constructorName = a.constructor.name;
    if (constructorName == 'DirectoryEntry') {
      return a.name.localeCompare(b.name);
    } else if (constructorName == 'Security') {
      let stringA = a.toString(definitionsService.getMarketDatabase());
      let stringB = b.toString(definitionsService.getMarketDatabase());
      return stringA.localeCompare(stringB);
    } else if (constructorName == 'CurrencyId') {
      let stringA = definitionsService.getCurrencyCode(a.toNumber());
      let stringB = definitionsService.getCurrencyCode(b.toNumber());
      return stringA.localeCompare(stringB);
    } else if (typeof a == 'string') {
      return a.localeCompare(b);
    } else {
      return a - b;
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
