import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';

class CommonView extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  componentDidUpdate() {
    $('#account-container').fadeIn({
      duration: Config.FADE_DURATION
    });
  }

  showCreateFailMessage() {
    $('#account-container .create-button-wrapper .create-message').text('Failed').css('display', 'inherit');
  }
}

export default CommonView;
