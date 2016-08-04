import httpConnectionManager from '../commons/http-connection-manager';
import ResultCodes from './result-codes.js';
import accountRoles from '../commons/account-roles';
const ResultCode = ResultCodes;

/** Spire admin client class */
class AdminClient {
  /** @private */
  logErrorAndThrow(xhr) {
    console.error('Unexpected error happened.');
    console.error(xhr);
    throw ResultCode.ERROR;
  }

  loadAccountRoles(directoryEntry) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/load_account_roles';
    let payload = {
      account: directoryEntry
    };

    return httpConnectionManager.send(apiPath, payload)
      .then(onResponse)
      .catch(this.logErrorAndThrow);

    function onResponse(roles) {
      return accountRoles.parse(roles);
    }
  }

  loadAccountProfile(directoryEntry) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/load_account_identity';
    let payload = {
      account: directoryEntry
    };

    return httpConnectionManager.send(apiPath, payload)
      .then(parseResponse)
      .catch(this.logErrorAndThrow);

    function parseResponse(response) {
      return {
        addressLineOne: response.address_line_one,
        addressLineTwo: response.address_line_two,
        addressLineThree: response.address_line_three,
        city: response.city,
        country: response.country,
        email: response.e_mail,
        firstName: response.first_name,
        lastLoginTime: response.last_login_time,
        lastName: response.last_name,
        photoId: response.photo_id,
        province: response.province,
        registrationTime: response.registration_time,
        userNotes: response.user_notes
      };
    }
  }
}

export default new AdminClient();