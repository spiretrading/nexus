import React from 'react';
import UpdatableView from 'components/common/UpdatableView';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  render() {
    return (
        <div id="skeleton-container">
        </div>
    );
  }
}

export default View;