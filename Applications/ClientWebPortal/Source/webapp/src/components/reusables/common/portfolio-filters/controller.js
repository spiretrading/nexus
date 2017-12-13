class Controller {
  constructor(react, componentModel) {
    this.componentModel = componentModel;
    this.onParametersSave = react.props.onSave;
    this.isInitialized = false;

    this.onFilterChange = this.onFilterChange.bind(this);
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  componentWillUpdate(model) {
    if (model != null) {
      if (!this.isInitialized) {
        this.isInitialized = true;
        overwriteMerge(this.componentModel, model);
      }

      this.view.setComponentModel(this.componentModel);
    }
  }

  onFilterChange(category, availableItems, selectedItems) {
    if (category === 'Group') {
      this.componentModel.groups = availableItems;
      this.componentModel.filter.groups = selectedItems;
    } else if (category === 'Currency') {
      this.componentModel.currencies = availableItems;
      this.componentModel.filter.currencies = selectedItems;
    } else if (category === 'Market') {
      this.componentModel.markets = availableItems;
      this.componentModel.filter.markets = selectedItems;
    } else if (category === 'Column') {
      this.componentModel.columns = availableItems;
      let clonedSelectedItems = clone(selectedItems);
      clonedSelectedItems.sort(this.compareId);
      this.componentModel.filter.columns = clonedSelectedItems;
    }
    this.view.update(this.componentModel);
  }

  /** @private */
  compareId(a, b) {
    return a.id - b.id;
  }

  update() {
    this.onParametersSave(clone(this.componentModel.filter));
  }
}

export default Controller;
