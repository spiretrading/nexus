import Model from 'utils/table-models/model';
import SignalManager from 'utils/signal-manager';
import DataChangeType from 'utils/table-models/model/data-change-type';
import TranslationModel from 'utils/table-models/translation-model';
import ValueComparer from 'utils/value-comparer';

export default class extends Model {
  constructor(sourceModel, columnSortOrders, valueComparer = new ValueComparer()) {
    super();
    this.columnSortOrders = columnSortOrders;
    this.valueComparer = valueComparer;
    this.signalManager = new SignalManager();
    this.translationModel = new TranslationModel(sourceModel);
    this.onDataChange = this.onDataChange.bind(this);
    this.dataChangeSubId = this.translationModel.addDataChangeListener(this.onDataChange);

    this.initialize();
  }

  getRowCount() {
    return this.translationModel.getRowCount();
  }

  getColumnCount() {
    return this.translationModel.getColumnCount();
  }

  getColumnName(x) {
    return this.translationModel.getColumnName(x);
  }

  getValueAt(x, y) {
    return this.translationModel.getValueAt(x, y);
  }

  addDataChangeListener(listener) {
    return this.signalManager.addListener(listener);
  }

  removeDataChangeListener(subId) {
    this.signalManager.removeListener(subId);
  }

  dispose() {
    this.translationModel.removeDataChangeListener(this.dataChangeSubId);
    this.translationModel.dispose();
  }

  /** @private */
  initialize() {
    let rowCount = this.translationModel.getRowCount();
    for (let i=1; i<rowCount; i++) {
      this.sortAddedData(i, i - 1);
    }
  }

  /** @private */
  sortAddedData(rowIndex, searchEndIndex) {
    let sortedIndex;
    if (this.columnSortOrders.length == 0) {
      sortedIndex = rowIndex;
    } else {
      let values = this.getValuesOfSortOrderColumns(rowIndex);
      sortedIndex = this.searchIndex(values, 0, searchEndIndex);
    }
    this.translationModel.moveRow(rowIndex, sortedIndex);
    return sortedIndex;
  }

  /** @private */
  getValuesOfSortOrderColumns(rowIndex) {
    let values = [];
    for (let i=0; i<this.columnSortOrders.length; i++) {
      let value = this.translationModel.getValueAt(this.columnSortOrders[i].index, rowIndex);
      values.push(value);
    }
    return values;
  }

  /** @private */
  searchIndex(values, initialStart, initialEnd, ignoredIndex = null) {
    if (ignoredIndex != null) {
      initialEnd--;
    }

    // binary search
    let startIndex = initialStart;
    let endIndex = initialEnd;
    while (startIndex + 2 <= endIndex) {
      // loop until we have the smallest possible array in our search
      let midIndex = Math.ceil((startIndex + endIndex) / 2);
      let midOriginalIndex = this.originalIndex(midIndex, ignoredIndex);
      let midValues = this.getValuesOfSortOrderColumns(midOriginalIndex);
      let compareResult = this.compareValuesBySortOrders(values, midValues);
      if (compareResult == 0) {
        return this.findHeadOfDuplicates(midOriginalIndex, ignoredIndex);
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
      if (ignoredIndex == null) {
        return this.findIndexWithoutIgnoredIndex(values, startIndex, endIndex);
      } else {
        return this.findIndexWithIgnoredIndex(values, startIndex, endIndex, ignoredIndex);
      }
    }
  }

  /** @private */
  originalIndex(index, ignoredIndex) {
    if (ignoredIndex == null) {
      return index;
    } else if (index >= ignoredIndex) {
      return index + 1;
    } else {
      return index;
    }
  }

  /** @private */
  findHeadOfDuplicates(startIndex, ignoredIndex) {
    let rowIndex = startIndex;
    let isDuplicate = true;
    while (isDuplicate) {
      if (rowIndex == 0) {
        break;
      }

      let currentValues = this.getValuesOfSortOrderColumns(rowIndex);
      let indexOneBefore = rowIndex - 1;
      if (indexOneBefore == ignoredIndex) {
        indexOneBefore--;
      }
      let valuesOneBefore = this.getValuesOfSortOrderColumns(indexOneBefore);
      let compareResult = this.compareValuesBySortOrders(currentValues, valuesOneBefore);
      if (compareResult == 0) {
        rowIndex--;
      } else {
        isDuplicate = false;
      }
    }

    if (ignoredIndex == null || ignoredIndex > startIndex) {
      return rowIndex;
    } else {
      return rowIndex - 1;
    }
  }

  /** @private */
  findIndexWithoutIgnoredIndex(values, startIndex, endIndex) {
    // compare value at start index
    let startValues = this.getValuesOfSortOrderColumns(startIndex);
    let compareToStartResult = this.compareValuesBySortOrders(values, startValues);
    if (compareToStartResult <= 0) {
      return startIndex;
    }

    // compare value at end index
    let endValues = this.getValuesOfSortOrderColumns(endIndex);
    let compareToEndResult = this.compareValuesBySortOrders(values, endValues);
    if (compareToEndResult <= 0) {
      return endIndex;
    }

    if (compareToEndResult > 0) {
      return endIndex + 1;
    }
  }

  /** @private */
  findIndexWithIgnoredIndex(values, startIndex, endIndex, ignoredIndex) {
    // compare value at start index
    let startOriginalIndex = this.originalIndex(startIndex, ignoredIndex);
    let startValues = this.getValuesOfSortOrderColumns(startOriginalIndex);
    let compareToStartResult = this.compareValuesBySortOrders(values, startValues);
    if (compareToStartResult <= 0) {
      if (ignoredIndex > startOriginalIndex) {
        return startOriginalIndex;
      } else {
        return startOriginalIndex - 1;
      }
    }

    // compare value at end index
    let endOriginalIndex = this.originalIndex(endIndex, ignoredIndex);
    let endValues = this.getValuesOfSortOrderColumns(endOriginalIndex);
    let compareToEndResult = this.compareValuesBySortOrders(values, endValues);
    if (compareToEndResult <= 0) {
      if (ignoredIndex < endOriginalIndex) {
        return endOriginalIndex - 1;
      } else {
        return endOriginalIndex;
      }
    } else {
      if (ignoredIndex < endOriginalIndex) {
        return endOriginalIndex;
      } else {
        return endOriginalIndex + 1;
      }
    }
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

  /** @private */
  onDataChange(dataChangeType, payload) {
    if (dataChangeType == DataChangeType.ADD) {
      this.handleDataAdd(payload);
    } else if (dataChangeType == DataChangeType.UPDATE) {
      this.handleDataUpdate(payload.index, payload.original);
    } else if (dataChangeType == DataChangeType.REMOVE) {
      this.signalManager.emitSignal(dataChangeType, payload);
    }
  }

  /** @private */
  handleDataAdd(rowIndex) {
    let newSortedRowIndex = this.sortAddedData(rowIndex, this.translationModel.getRowCount() - 1);
    this.signalManager.emitSignal(DataChangeType.ADD, newSortedRowIndex);
  }

  /** @private */
  shouldSort(currentSortColumnValues, priorRowValues) {
    let priorSortColumnValues = [];
    for (let i=0; i<this.columnSortOrders; i++) {
      priorSortColumnValues.push(this.columnSortOrders[i].index);
    }
    if (this.compareValuesBySortOrders(priorSortColumnValues, currentSortColumnValues) != 0) {
      return true;
    } else {
      return false;
    }
  }

  /** @private */
  handleDataUpdate(rowIndex, previousRowValues) {
    let currentSortColumnValues = this.getValuesOfSortOrderColumns(rowIndex);
    if (this.shouldSort(currentSortColumnValues, previousRowValues)) {
      let newSortedIndex = this.searchIndex(
        currentSortColumnValues,
        0,
        this.translationModel.getRowCount() - 1,
        rowIndex
      );
      if (rowIndex != newSortedIndex) {
        this.translationModel.moveRow(rowIndex, newSortedIndex);
        this.signalManager.emitSignal(DataChangeType.MOVE, Object.freeze({
          from: rowIndex,
          to: newSortedIndex
        }));
      }

      this.signalManager.emitSignal(DataChangeType.UPDATE, {
        index: newSortedIndex,
        original: previousRowValues
      });
    } else {
      this.signalManager.emitSignal(DataChangeType.UPDATE, {
        index: rowIndex,
        original: previousRowValues
      });
    }
  }
}
