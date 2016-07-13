import View from './view';
import {browserHistory} from 'react-router/es6';
import userService from 'services/user';
import ResultCode from 'utils/spire-clients/result-codes';

/** Login form controller */
class Controller {
  constructor(react) {
    this.componentModel = {
      isWaiting: false,
      loginResultCode: 0
    };
    this.view = new View(react, this, cloneObject(this.componentModel));
  }

  getView() {
    return this.view;
  }

  login(userId, password) {
    this.componentModel.isWaiting = true;
    this.view.update(cloneObject(this.componentModel));

    userService.login(userId, password)
      .then(onResult.bind(this));

    function onResult(resultCode) {
      if (resultCode === ResultCode.SUCCESS) {
        browserHistory.push('/searchProfiles')
      }
      else {
        this.componentModel.isWaiting = false;
        this.componentModel.loginResultCode = resultCode;
        this.view.update(cloneObject(this.componentModel));
      }
    }
  }
}

export default Controller;