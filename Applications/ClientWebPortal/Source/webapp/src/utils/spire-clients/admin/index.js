import httpConnectionManager from '../commons/http-connection-manager';
import ResultCodes from './result-codes.js';
import accountRoles from '../commons/account-roles';
const ResultCode = ResultCodes;

/** Spire admin client class */
class AdminClient {
  /** @private */
  logErrorAndThrow(xhr) {
    console.error('Unexpected error happened.');
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
}

export default new AdminClient();