import spireClient from 'utils/spire-client';
import ResultCode from 'utils/spire-client/result-codes.js';

class UserService {
  constructor() {
    this.userRole;
  }

  login(userId, password) {
    let resultCode = null;

    return spireClient.login(userId, password)
        .then(onLoginResponse)
        .then(onUserRoleResponse.bind(this))
        .catch(onException);


    function onLoginResponse(aResultCode) {
      resultCode = aResultCode;
      if (resultCode === ResultCode.Fail) {
        throw new Error();
      }

      return new Promise((resolve, reject) => {
        spireClient.getUserRole(userId)
            .then((role) => {
              resolve(role);
            })
            .catch(() => {
              reject();
            });
      });
    }

    function onUserRoleResponse(role) {
      this.userRole = role;
      EventBus.publish(Event.Application.LoggedIn, {});
      return resultCode;
    }

    function onException() {
      if (resultCode === ResultCode.Fail) {
        return resultCode;
      }
      else {
        throw new Error();
      }
    }
  }
}

export default new UserService();