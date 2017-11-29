import Model from 'utils/table-models/model';
import SignalManager from 'utils/signal-manager';
import DataChangeType from 'utils/table-models/model/data-change-type';
import ValueComparer from 'utils/value-comparer';

export default class extends Model {
  constructor(sourceModel, filter, valueComparer = new ValueComparer()) {
    super();
    this.sourceModel = sourceModel;
    this.filter = filter;
    this.valueComparer = valueComparer;
    this.signalManager = new SignalManager();
    this.onDataChange = this.onDataChange.bind(this);
    this.dataChangeSubId = this.sourceModel.addDataChangeListener(this.onDataChange);

    this.sourceToProjectionRows = [];
    this.projectionToSourceRows = [];

    this.initialize(filter);
  }

  getRowCount() {
    return this.projectionToSourceRows.length;
  }

  getColumnCount() {
    return this.sourceModel.getColumnCount();
  }

  getColumnName(x) {
    return this.sourceModel.getColumnName(x);
  }

  getValueAt(x, y) {
    return this.sourceModel.getValueAt(x, this.projectionToSourceRows[y]);
  }

  addDataChangeListener(listener) {
    return this.signalManager.addListener(listener);
  }

  removeDataChangeListener(subId) {
    this.signalManager.removeListener(subId);
  }

  dispose() {
    this.sourceModel.removeDataChangeListener(this.dataChangeSubId);
  }

  /** @private */
  initialize(filter) {
    let sourceRowCount = this.sourceModel.getRowCount();
    for (let y=0; y<sourceRowCount; y++) {
      if (this.testFilter(y)) {
        this.projectionToSourceRows.push(y);
        this.sourceToProjectionRows[y] = this.projectionToSourceRows.length - 1;
      }
    }
  }

  /** @private */
  testFilter(sourceRowIndex) {
    for (let i=0; i<this.filter.length; i++) {
      let sourceValue = this.sourceModel.getValueAt(this.filter[i].index, sourceRowIndex);
      if (this.valueComparer.compare(sourceValue, this.filter[i].value) != 0) {
        return false;
      }
    }
    return true;
  }

  /** @private */
  onDataChange(dataChangeType, payload) {
    if (dataChangeType == DataChangeType.ADD) {
      this.handleDataAdd(payload);
    } else if (dataChangeType == DataChangeType.REMOVE) {
      this.handleDataRemove(payload);
    } else if (dataChangeType == DataChangeType.UPDATE) {
      this.handleDataUpdate(payload);
    } else if (dataChangeType == DataChangeType.MOVE) {
      this.signalManager.emitSignal(DataChangeType.MOVE, payload);
    }
  }

  /** @private */
  handleDataAdd(payload) {
    if (this.testFilter(payload)) {
      this.projectionToSourceRows.push(payload);
      this.sourceToProjectionRows[payload] = this.projectionToSourceRows.length - 1;
      this.signalManager.emitSignal(DataChangeType.ADD, this.projectionToSourceRows.length - 1);
    }
  }

  /** @private */
  handleDataRemove(payload) {
    let removedSrcIndex = payload.index;
    let projectionIndex = this.projectionToSourceRows.indexOf(removedSrcIndex);
    this.sourceToProjectionRows.splice(removedSrcIndex, 1);
    if (projectionIndex > -1) {
      this.projectionToSourceRows.splice(projectionIndex, 1);
    }
    let decrementStartIndex = this.projectionToSourceRows.findIndex(function(element) {
      return element > removedSrcIndex;
    });
    for (let i=decrementStartIndex; i<this.projectionToSourceRows.length; i++) {
      this.projectionToSourceRows[i]--;
      this.sourceToProjectionRows[this.projectionToSourceRows[i]] = i;
      this.sourceToProjectionRows[this.projectionToSourceRows[i] + 1] = undefined;
    }
    if (projectionIndex > -1) {
      this.signalManager.emitSignal(DataChangeType.REMOVE, {
        index: projectionIndex,
        row: payload.row
      });
    }
  }

  /** @private */
  handleDataUpdate(payload) {
    let projectionIndex = this.projectionToSourceRows.indexOf(payload.index);
    let isInProjection = projectionIndex > -1;
    let shouldBeIncluded = this.testFilter(payload.index);
    if (!isInProjection && shouldBeIncluded) {
      // add to projection
      let projectionIndexToInsert = this.projectionToSourceRows.findIndex(function(element) {
        return element > payload.index;
      });
      if (projectionIndexToInsert == -1) {
        projectionIndexToInsert = this.projectionToSourceRows.length;
      }
      this.projectionToSourceRows.splice(projectionIndexToInsert, 0, payload.index);
      this.sourceToProjectionRows[payload.index] = projectionIndexToInsert;
      for (let i=projectionIndexToInsert; i<this.projectionToSourceRows.length; i++) {
        this.sourceToProjectionRows[this.projectionToSourceRows[i]] = i;
      }
      this.signalManager.emitSignal(DataChangeType.ADD, projectionIndexToInsert);
    } else if (isInProjection  && !shouldBeIncluded) {
      // remove from projection
      this.projectionToSourceRows.splice(projectionIndex, 1);
      this.sourceToProjectionRows[payload.index] = undefined;
      for (let i=projectionIndex; i<this.projectionToSourceRows.length; i++) {
        this.sourceToProjectionRows[this.projectionToSourceRows[i]] = i;
      }
      this.signalManager.emitSignal(DataChangeType.REMOVE, {
        index: projectionIndex,
        row: payload.original
      });
    } else if (isInProjection) {
      this.signalManager.emitSignal(DataChangeType.UPDATE, {
        index: this.sourceToProjectionRows[payload.index],
        original: payload.original
      });
    }
  }
}
