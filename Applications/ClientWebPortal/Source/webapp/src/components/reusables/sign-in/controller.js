import View from './view';
import {browserHistory} from 'react-router/es6';
import userService from 'services/user';
import ResultCode from 'utils/spire-clients/service-locator/result-codes';

/** Signin form controller */
class Controller {
  constructor(react) {
    this.componentModel = {
      isLoading: false,
      loginResultCode: 0
    };
    this.view = new View(react, this, cloneObject(this.componentModel));
  }

  getView() {
    return this.view;
  }

  componentDidMount() {
    this.view.initialize();
  }

  componentWillUnmount() {
    this.view.dispose();
  }

  signIn(userId, password) {
    this.componentModel.isLoading = true;
    this.view.update(cloneObject(this.componentModel));

    userService.signIn(userId, password)
      .then(onResult.bind(this));

    function onResult(resultCode) {
      if (resultCode === ResultCode.SUCCESS) {
        browserHistory.push('/searchProfiles')
      } else {
        this.componentModel.isLoading = false;
        this.componentModel.signInResultCode = resultCode;
        this.view.update(cloneObject(this.componentModel));
      }
    }
  }
}

export default Controller;