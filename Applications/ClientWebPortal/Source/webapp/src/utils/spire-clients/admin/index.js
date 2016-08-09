import httpConnectionManager from '../commons/http-connection-manager';
import ResultCodes from './result-codes.js';
import accountRoles from '../commons/account-roles';
const ResultCode = ResultCodes;

/** Spire admin client class */
class AdminClient {
  /** @private */
  logErrorAndThrow(xhr) {
    console.error('Spire Admin Client: Unexpected error happened.');
    console.debug(xhr);
    throw ResultCode.ERROR;
  }

  loadAccountRoles(directoryEntry) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/load_account_roles';
    let payload = {
      account: directoryEntry
    };

    return httpConnectionManager.send(apiPath, payload, true)
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

    return httpConnectionManager.send(apiPath, payload, true)
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

  storeAccountIdentity(directoryEntry, accountIdentity) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/store_account_identity';
    let payload = {
      account: directoryEntry,
      identity: {
        address_line_one: accountIdentity.addressLineOne,
        address_line_two: accountIdentity.addressLineTwo,
        address_line_three: accountIdentity.addressLineThree,
        city: accountIdentity.city,
        country: accountIdentity.country,
        e_mail: accountIdentity.email,
        first_name: accountIdentity.firstName,
        last_login_time: accountIdentity.lastLoginTime,
        last_name: accountIdentity.lastName,
        photo_id: accountIdentity.photoId,
        province: accountIdentity.province,
        registration_time: accountIdentity.registrationTime,
        user_notes: accountIdentity.userNotes
      }
    };

    return httpConnectionManager.send(apiPath, payload, false)
      .catch(this.logErrorAndThrow);
  }
}

export default new AdminClient();