import NotifyChangeModel from 'utils/table-models/notify-change-model';

export default class extends NotifyChangeModel {
  constructor(sourceModel) {
    super();

    if (sourceModel == null) {
      throw new TypeError('Source model cannot be null');
    }

    this.sourceModel = sourceModel;
  }
}
