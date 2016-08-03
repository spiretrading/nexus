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
        addressOne: response.address_line_one,
        addressTwo: response.address_line_two,
        addressThree: response.address_line_three,
        city: response.city,
        country: response.country,
        email: response.e_mail,
        firstName: response.first_name,
        lastSigninDateTime: response.last_login_time,
        lastName: response.last_name,
        picture: response.photo_id,
        province: response.province,
        registrationDateTime: response.registration_time,
        userName: directoryEntry.name,
        userNotes: response.user_notes
      };
    }
  }
}

export default new AdminClient();