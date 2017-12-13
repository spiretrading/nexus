import Model from 'utils/table-models/model';
import SignalManager from 'utils/signal-manager';
import DataChangeType from 'utils/table-models/model/data-change-type';

export default class extends Model {
  constructor(sourceModel, rowTranslation = null, columnTranslation = null) {
    super();
    this.sourceModel = sourceModel;
    this.signalManager = new SignalManager();
    this.onDataChange = this.onDataChange.bind(this);
    this.dataChangeSubId = this.sourceModel.addDataChangeListener(this.onDataChange);

    this.srcToTransRow = [];
    this.transToSrcRow = [];
    this.srcToTransColumn = [];
    this.transToSrcColumn = [];

    this.initialize(rowTranslation, columnTranslation);
  }

  getRowCount() {
    return this.sourceModel.getRowCount();
  }

  getColumnCount() {
    return this.sourceModel.getColumnCount();
  }

  getColumnName(columnIndex) {
    return this.sourceModel.getColumnName(this.transToSrcColumn[columnIndex]);
  }

  getValueAt(x, y) {
    return this.sourceModel.getValueAt(
      this.transToSrcColumn[x],
      this.transToSrcRow[y]
    );
  }

  moveRow(from, to) {
    let next = this.transToSrcRow[from];
    if (from < to) {
      for (let i=to; i>=from; i--) {
        next = this.updateRowMaps(i, next);
      }
    } else if (from > to) {
      for (let i=to; i<=from; i++) {
        next = this.updateRowMaps(i, next);
      }
    }
  }

  moveColumn(from, to) {
    let next = this.transToSrcColumn[from];
    if (from < to) {
      for (let i=to; i>=from; i--) {
        next = this.updateColumnMaps(i, next);
      }
    } else if (from > to) {
      for (let i=to; i<=from; i++) {
        next = this.updateColumnMaps(i, next);
      }
    }
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
  initialize(rowTranslation, columnTranslation) {
    // initialize row
    if (rowTranslation == null) {
      for (let i=0; i<this.getRowCount(); i++) {
        this.srcToTransRow.push(i);
        this.transToSrcRow.push(i);
      }
    } else {
      this.transToSrcRow = rowTranslation.slice();
      for (let i=0; i<this.transToSrcRow.length; i++) {
        this.srcToTransRow[this.transToSrcRow[i]] = i;
      }
    }

    // initialize column
    if (columnTranslation == null) {
      for (let i=0; i<this.getColumnCount(); i++) {
        this.srcToTransColumn.push(i);
        this.transToSrcColumn.push(i);
      }
    } else {
      this.transToSrcColumn = columnTranslation.slice();
      for (let i=0; i<this.transToSrcColumn.length; i++) {
        this.srcToTransColumn[this.transToSrcColumn[i]] = i;
      }
    }
  }

  /** @private */
  updateRowMaps(transIndex, srcIndex) {
    let oldTransIndex = this.transToSrcRow[transIndex];
    this.transToSrcRow[transIndex] = srcIndex;
    this.srcToTransRow[srcIndex] = transIndex;
    return oldTransIndex;
  }

  /** @private */
  updateColumnMaps(transIndex, srcIndex) {
    let oldTransIndex = this.transToSrcColumn[transIndex];
    this.transToSrcColumn[transIndex] = srcIndex;
    this.srcToTransColumn[srcIndex] = transIndex;
    return oldTransIndex;
  }

  /** @private */
  onDataChange(dataChangeType, payload) {
    // move is not handled as it translates in complex, multi-part translated move signals
    if (dataChangeType == DataChangeType.ADD) {
      this.srcToTransRow.push(payload);
      this.transToSrcRow.push(payload);
      this.signalManager.emitSignal(
        dataChangeType,
        this.srcToTransRow[payload]
      );
    } else if (dataChangeType == DataChangeType.REMOVE) {
      let transIndex = this.srcToTransRow[payload.index];
      this.srcToTransRow.splice(payload.index, 1);
      this.transToSrcRow.splice(transIndex, 1);
      for (let i=0; i<this.srcToTransRow.length; i++) {
        if (this.srcToTransRow[i] > transIndex) {
          this.srcToTransRow[i]--;
        }
        if (this.transToSrcRow[i] > payload.index) {
          this.transToSrcRow[i]--;
        }
      }
      this.signalManager.emitSignal(dataChangeType, {
        index: transIndex,
        row: Object.freeze(this.transformRowData(payload.row))
      });
    } else if (dataChangeType == DataChangeType.UPDATE) {
      this.signalManager.emitSignal(dataChangeType, {
        index: this.srcToTransRow[payload.index],
        original: Object.freeze(this.transformRowData(payload.original))
      });
    }
  }

  /** @private */
  transformRowData(row) {
    let transRow = [];
    for (let i=0; i<this.srcToTransColumn.length; i++) {
      transRow[this.srcToTransColumn[i]] = row[i];
    }
    return transRow;
  }
}
