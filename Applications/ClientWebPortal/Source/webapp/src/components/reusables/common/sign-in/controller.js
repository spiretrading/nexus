import View from './view';
import userService from 'services/user';
import definitionsService from 'services/definitions';
import ResultCode from 'services/user/result-codes';
import store from 'store';
import LocalstorageKey from 'commons/localstorage-keys';
import {browserHistory} from 'react-router/es6';

class Controller {
  constructor(react) {
    this.componentModel = {
      isLoading: false,
      loginResultCode: 0
    };
    this.view = new View(react, this, this.componentModel);
  }

  /** @private */
  initializeApp() {
    return Promise.all([
      definitionsService.initialize.apply(definitionsService)
    ]);
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

  signIn(userName, password) {
    this.componentModel.isLoading = true;
    this.view.update(this.componentModel);

    userService.signIn(userName, password)
      .then(onResult.bind(this));

    function onResult(resultCode) {
      store.remove(LocalstorageKey.LAST_VISITED_PATH);
      if (resultCode === ResultCode.SUCCESS) {
        this.initializeApp().then(() => {
          let lastVisitedPath = store.get(LocalstorageKey.LAST_VISITED_PATH);
          if (lastVisitedPath != null &&
            (lastVisitedPath != '/' || lastVisitedPath != 'profile-account') &&
            userService.isAuthorizedPath(lastVisitedPath)){
            browserHistory.push(lastVisitedPath);
          } else {
            browserHistory.push('profile-account', userService.getDirectoryEntry());
          }
        });
      } else {
        this.componentModel.isLoading = false;
        this.componentModel.signInResultCode = resultCode;
        this.view.update(this.componentModel);
      }
    }
  }
}

export default Controller;
