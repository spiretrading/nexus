import React from 'react';
import deviceDetector from 'utils/device-detector';
import UpdatableView from 'commons/updatable-view';
import ResultCode from 'utils/spire-clients/result-codes';

/** Login form view */
class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.isLoginAttempted = false;
    this.onLoginBtnClick = this.onLoginBtnClick.bind(this);
  }

  /** @private */
  onKeyPress(e) {
    if (e.key === 'Enter') {
      this.getDetailsAndLogin();
    }
  }

  /** @private */
  onLoginBtnClick() {
    this.getDetailsAndLogin();
  }

  /** @private */
  getDetailsAndLogin() {
    this.isLoginAttempted = true;
    let userId = $('#login-container .username-input').val().trim();
    let password = $('#login-container .password-input').val().trim();
    this.controller.login(userId, password);
  }

  componentDidUpdate() {
    if (this.componentModel.isLoginSuccess) {
      $('#login-container .username-input').val('');
      $('#login-container .password-input').val('');
    }
  }

  render() {
    let staticLogoStyle = {};
    if (this.componentModel.isLoading) {
      staticLogoStyle.display = "none";
    }

    let logos;
    if (deviceDetector.isInternetExplorer()) {
      logos = <div className="logo-container">
        <img className="preloader" src="images/white-logo-loading@2x.gif"/>
        <img className="static" src="images/white-logo@2x.png" style={staticLogoStyle}/>
      </div>
    } else {
      logos = <div className="logo-container">
        <object className="preloader" data="images/spireloading.svg" type="image/svg+xml"/>
        <object className="static" data="images/spire_logo.svg" type="image/svg+xml" style={staticLogoStyle}/>
      </div>
    }

    let message;
    if (this.isLoginAttempted && !this.componentModel.isWaiting) {
      if (this.componentModel.loginResultCode == ResultCode.FAIL) {
        message = "Invalid username or password.";
      }
    }

    return (
      <div id="login-container">
        {logos}
        <input type="text" className="username-input" defaultValue="" placeholder="Username"
               onKeyPress={this.onKeyPress.bind(this)}/>
        <input type="password" className="password-input" defaultValue="" placeholder="Password"
               onKeyPress={this.onKeyPress.bind(this)}/>
        <div className="white-btn login-btn" onClick={this.onLoginBtnClick}>Login</div>
        <div className="message">{message}</div>
      </div>
    );
  }
}

export default View;