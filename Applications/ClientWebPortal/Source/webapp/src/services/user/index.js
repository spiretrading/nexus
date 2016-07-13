import spireClient from 'utils/spire-clients';
import ResultCodes from 'utils/spire-clients/result-codes.js';
const ResultCode = ResultCodes;

/** Centralized user related states and service actions */
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
        throw resultCode;
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

    function onException(resultCode) {
      if (resultCode === ResultCode.Fail) {
        return resultCode;
      }
      else {
        throw ResultCode.ERROR;
      }
    }
  }
}

export default new UserService();