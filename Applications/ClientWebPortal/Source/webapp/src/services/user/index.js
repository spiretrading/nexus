import spireClient from 'utils/spire-clients';
import ResultCode from 'utils/spire-clients/result-codes.js';

/** Centralized user related states and service actions */
class UserService {
  constructor() {
    this.userRole;
  }

  signIn(userId, password) {
    let resultCode = null;

    return spireClient.signIn(userId, password)
      .then(onSignInResponse)
      .then(onUserRoleResponse.bind(this))
      .catch(onException);

    function onSignInResponse(aResultCode) {
      resultCode = aResultCode;
      if (resultCode === ResultCode.FAIL) {
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
      EventBus.publish(Event.Application.SIGNED_IN, {});
      return resultCode;
    }

    function onException(resultCode) {
      if (resultCode === ResultCode.FAIL) {
        return resultCode;
      } else {
        return ResultCode.ERROR;
      }
    }
  }

  signOut() {
    spireClient.signOut();
  }
}

export default new UserService();