import React from 'react';

/** Updatable view allows controllers to invoke update with a snapshot of component model */
class UpdatableView {
  constructor(react, controller, componentModel) {
    this.react = react;
    this.controller = controller;
    this.componentModel = componentModel;
    this.render = this.render.bind(this);
  }

  update(newComponentModel) {
    this.componentModel = newComponentModel;
    this.react.forceUpdate();
  }

  componentDidUpdate() {
  }
}

export default UpdatableView;