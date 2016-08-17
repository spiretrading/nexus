import React from 'react';

/** Updatable view allows controllers to invoke update with a snapshot of component model */
class UpdatableView {
  constructor(react, controller, componentModel) {
    this.react = react;
    this.controller = controller;
    this.componentModel = cloneObject(componentModel);
  }

  update(newComponentModel) {
    this.componentModel = cloneObject(newComponentModel);
    this.react.forceUpdate();
  }

  setComponentModel(newComponentModel) {
    this.componentModel = newComponentModel;
  }

  componentDidUpdate() {
  }
}

export default UpdatableView;