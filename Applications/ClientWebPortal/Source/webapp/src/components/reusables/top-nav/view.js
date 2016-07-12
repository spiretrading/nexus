import React from 'react';
import UpdatableView from 'commons/updatable-view';

/** Top nav view */
class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  render() {
    return (
        <div id="top-nav-container">
        </div>
    );
  }
}

export default View;