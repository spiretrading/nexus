import './style.scss';
import React from 'react';
import deviceDetector from 'utils/device-detector';
import UpdatableView from 'commons/updatable-view';
import ResultCode from 'services/user/result-codes';

/** Signin form view */
class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.isSignInAttempted = false;
    this.onSignInBtnClick = this.onSignInBtnClick.bind(this);
  }

  /** @private */
  onKeyPress(e) {
    if (e.key === 'Enter') {
      this.getDetailsAndSignIn();
    }
  }

  /** @private */
  onSignInBtnClick() {
    this.getDetailsAndSignIn();
  }

  /** @private */
  getDetailsAndSignIn() {
    this.isSignInAttempted = true;
    let userName = $('#signin-container .username-input').val().trim();
    let password = $('#signin-container .password-input').val().trim();
    this.controller.signIn(userName, password);
  }

  initialize() {
    $(document).on('keypress', (e) => {
      let $usernameInput = $('#signin-container .username-input');
      if (!$usernameInput.is(':focus') &&
        $usernameInput.val().trim().length === 0 && !$('#signin-container .password-input').is(':focus')) {
        $usernameInput.focus();
        setTimeout(() => {
          // only firefox requires this extra check
          if ($usernameInput.val().length == 0) {
            $usernameInput.val(String.fromCharCode(e.which));
          }
        }, 0);
      }
    });
  }

  dispose() {
    $(document).off('keypress');
  }

  componentDidUpdate() {
    if (this.componentModel.isSignInSuccess) {
      $('#signin-container .username-input').val('');
      $('#signin-container .password-input').val('');
    }

    setTimeout(() => {
      if (!this.componentModel.isLoading) {
        $('#signin-container .preloader').css('opacity', 0);
      }
    }, 200);
  }

  render() {
    let staticLogoStyle = {};
    let preloaderStyle = {};
    if (this.componentModel.isLoading) {
      staticLogoStyle.opacity = '0';
      preloaderStyle.opacity = '1';
    } else {
      staticLogoStyle.opacity = '1';
    }

    let logos;
    if (deviceDetector.isInternetExplorer()) {
      logos = <div className="logo-container">
        <img className="preloader" src="images/inverted_logo_animation.gif" style={preloaderStyle}/>
        <img className="static" src="images/inverted_logo.png" style={staticLogoStyle}/>
      </div>
    } else {
      logos = <div className="logo-container">
        <object className="preloader" data="images/spire_loading_inverted.svg" type="image/svg+xml"
                style={preloaderStyle}/>
        <object className="static" data="images/spire_logo_inverted.svg" type="image/svg+xml" style={staticLogoStyle}/>
      </div>
    }

    let message;
    if (this.isSignInAttempted && !this.componentModel.isWaiting) {
      if (this.componentModel.signInResultCode == ResultCode.FAIL) {
        message = 'Invalid username or password.';
      }
    }

    return (
      <div id="signin-container">
        {logos}
        <input type="text" className="username-input" defaultValue="" placeholder="Username"
               onKeyPress={this.onKeyPress.bind(this)}/>
        <input type="password" className="password-input" defaultValue="" placeholder="Password"
               onKeyPress={this.onKeyPress.bind(this)}/>
        <div className="white-btn signin-btn" onClick={this.onSignInBtnClick}>Sign In</div>
        <div className="message">{message}</div>
      </div>
    );
  }
}

export default View;
