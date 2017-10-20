import HashMap from 'hashmap';
import uuid from 'uuid';

export default class {
  constructor(sourceModel) {
    this.sourceModel = sourceModel;
    this.dataChangeListeners = new HashMap();
  }

  setSourceModel(sourceModel) {
    // unsub from previous source model
    this.sourceModel.removeDataChangeListener(this.dataChangeSubId);

    // sub new source model
    this.sourceModel = sourceModel;
    this.dataChangeSubId = this.sourceModel.addDataChangeListener(this.onDataChange);
  }

  addDataChangeListener(listener) {
    let subId = uuid.v4();
    this.dataChangeListeners.set(subId, listener);
    return subId;
  }

  removeDataChangeListener(subId) {
    this.dataChangeListeners.remove(subId);
  }

  dispose() {
    this.sourceModel.removeDataChangeListener(this.dataChangeSubId);
  }
}
