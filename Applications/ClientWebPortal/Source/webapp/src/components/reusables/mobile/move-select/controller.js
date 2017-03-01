class Controller {
  constructor(react, componentModel) {
    this.componentModel = clone(componentModel);
    this.onSelectedChange = react.props.onChange;
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  componentDidMount() {
  }

  componentWillUpdate(model) {
    if (model != null) {
      overwriteMerge(this.componentModel, model);
      this.view.setComponentModel(this.componentModel);
    }
    this.view.deselectAll.apply(this.view);
  }

  onChange(availableItems, selectedItems) {
    this.onSelectedChange(selectedItems);
    this.componentModel.availableItems = availableItems;
    this.view.update(this.componentModel);
  }

  /** @private */
  compareAlphabetically(a, b) {
    if (a.name.toLowerCase() < b.name.toLowerCase()) return -1;
    if (a.name.toLowerCase() > b.name.toLowerCase()) return 1;
    return 0;
  }

  addItems(addingItems) {
    for (let i=this.componentModel.availableItems.length - 1; i>=0; i--) {
      let currentId = this.componentModel.availableItems[i].id.toString();
      if (addingItems.indexOf(currentId) >= 0) {
        let removedItem = this.componentModel.availableItems.splice(i, 1);
        this.componentModel.selectedItems.push(removedItem[0]);
      }
    }

    this.componentModel.availableItems.sort(this.compareAlphabetically);
    this.componentModel.selectedItems.sort(this.compareAlphabetically);

    this.onSelectedChange(this.componentModel.availableItems, this.componentModel.selectedItems);
  }

  addAllItems() {
    let itemsToAdd = this.componentModel.availableItems;
    for (let i=0; i<itemsToAdd.length; i++) {
      this.componentModel.selectedItems.push(itemsToAdd[i]);
    }
    this.componentModel.availableItems = [];
    this.componentModel.selectedItems.sort(this.compareAlphabetically);
    this.onSelectedChange(this.componentModel.availableItems, this.componentModel.selectedItems);
  }

  removeItems(removingItems) {
    for (let i=this.componentModel.selectedItems.length - 1; i>=0; i--) {
      let currentId = this.componentModel.selectedItems[i].id.toString();
      if (removingItems.indexOf(currentId) >= 0) {
        let removedItem = this.componentModel.selectedItems.splice(i, 1);
        this.componentModel.availableItems.push(removedItem[0]);
      }
    }
    this.componentModel.availableItems.sort(this.compareAlphabetically);
    this.componentModel.selectedItems.sort(this.compareAlphabetically);

    this.onSelectedChange(this.componentModel.availableItems, this.componentModel.selectedItems);
  }

  removeAllItems() {
    let itemsToRemove = this.componentModel.selectedItems;
    for (let i=0; i<itemsToRemove.length; i++) {
      this.componentModel.availableItems.push(itemsToRemove[i]);
    }
    this.componentModel.selectedItems = [];
    this.componentModel.availableItems.sort(this.compareAlphabetically);
    this.onSelectedChange(this.componentModel.availableItems, this.componentModel.selectedItems);
  }
}

export default Controller;
