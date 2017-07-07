import React from 'react';

/** Updatable view allows controllers to invoke update with a snapshot of component model */
class UpdatableView {
  constructor(react, controller, componentModel) {
    this.react = react;
    this.controller = controller;
    this.componentModel = clone(componentModel);

    this.update = this.update.bind(this);
    this.setComponentModel = this.setComponentModel.bind(this);
  }

  update(newComponentModel) {
    if ($.isArray(this.componentModel)) {
      this.componentModel = newComponentModel;
    } else if (typeof this.componentModel !== 'object') {
      this.componentModel = newComponentModel;
    } else {
      overwriteMerge(this.componentModel, newComponentModel);
    }
    this.react.forceUpdate();
  }

  setComponentModel(newComponentModel) {
    this.componentModel = newComponentModel;
  }

  componentDidUpdate() {
  }
}

export default UpdatableView;
