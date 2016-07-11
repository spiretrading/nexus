import React from 'react';
import deviceDetector from 'utils/device-detector';

function View(react, controller, componentModel){
    // PRIVATE
    let isLoginAttempted = false;

    function onLoginBtnClick() {
        isLoginAttempted = true;
        let userId = $('#login-container .username-input').val().trim();
        let password = $('#login-container .password-input').val().trim();
        controller.login(userId, password);
    };

    // PUBLIC
    this.update = (newComponentModel) => {
        componentModel = newComponentModel;
        react.forceUpdate();
    }

    this.componentDidUpdate = () => {
        if (componentModel.isLoginSuccess){
            $('#login-container .username-input').val('');
            $('#login-container .password-input').val('');
        }
    }

    this.render = () => {
        let staticLogoStyle = {};
        if (componentModel.isLoading){
            staticLogoStyle.display = "none";
        }

        let logos;
        if(deviceDetector.isInternetExplorer()){
            logos = <div className="logo-container">
                <img className="preloader" src="images/white-logo-loading@2x.gif" />
                <img className="static" src="images/white-logo@2x.png" style={staticLogoStyle} />
            </div>
        }
        else{
            logos = <div className="logo-container">
                <object className="preloader" data="images/spireloading.svg" type="image/svg+xml" />
                <object className="static" data="images/spire_logo.svg" type="image/svg+xml" style={staticLogoStyle} />
            </div>
        }

        let message;
        if (isLoginAttempted && !componentModel.isWaiting && !componentModel.isLoginSuccess){
            message = "Invalid username or password.";
        }

        return (
            <div id="login-container">
                {logos}
                <input type="text" className="username-input" defaultValue="" placeholder="Username"  />
                <input type="password" className="password-input" defaultValue="" placeholder="Password" />
                <div className="white-btn login-btn" onClick={onLoginBtnClick}>Login</div>
                <div className="message">{message}</div>
            </div>
        );
    }
}

export default View;