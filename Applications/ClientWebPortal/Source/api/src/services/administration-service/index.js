import httpConnectionManager from '../commons/http-connection-manager';
import accountRoles from '../commons/account-roles';
import AccountIdentity from './account-identity';

/** Spire admin client class */
class Admin {
  /** @private */
  logErrorAndThrow(xhr) {
    let errorMessage = 'Spire Admin Client: Unexpected error happened.';
    console.error(errorMessage);
    console.error(xhr);
    throw errorMessage;
  }

  loadAccountRoles(directoryEntry) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/load_account_roles';
    let payload = {
      account: directoryEntry.toData()
    };

    return httpConnectionManager.send(apiPath, payload, true)
      .then(onResponse)
      .catch(this.logErrorAndThrow);

    function onResponse(roles) {
      let accRoles = accountRoles.parse(roles)
      accRoles.id = directoryEntry.id;
      return accRoles;
    }
  }

  loadAccountIdentity(directoryEntry) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/load_account_identity';
    let payload = {
      account: directoryEntry.toData()
    };

    return httpConnectionManager.send(apiPath, payload, true)
      .then(parseResponse)
      .catch(this.logErrorAndThrow);

    function parseResponse(response) {
      return AccountIdentity.fromData(response);
    }
  }

  storeAccountIdentity(directoryEntry, accountIdentity) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/store_account_identity';
    let payload = {
      account: directoryEntry.toData(),
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

  loadRiskParameters(directoryEntry) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/load_risk_parameters';
    let payload = {
      account: directoryEntry.toData()
    };

    return httpConnectionManager.send(apiPath, payload, true)
      .then(parseResponse)
      .catch(this.logErrorAndThrow);

    function parseResponse(response) {
      return {
        currency: response.currency,
        netLoss: response.net_loss,
        buyingPower: response.buying_power,
        transitionTime: response.transition_time
      };
    }
  }

  storeRiskParameters(directoryEntry, riskParameters) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/store_risk_parameters';
    let payload = {
      account: directoryEntry.toData(),
      risk_parameters: {
        buying_power: riskParameters.buyingPower,
        currency: riskParameters.currency,
        net_loss: riskParameters.netLoss,
        transition_time: riskParameters.transitionTime
      }
    };

    return httpConnectionManager.send(apiPath, payload, false)
      .catch(this.logErrorAndThrow);
  }

  loadEntitlementsData() {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/load_entitlements_database';
    return httpConnectionManager.send(apiPath, null, true)
      .then(onResponse)
      .catch(this.logErrorAndThrow);

    function onResponse(response) {
      return response.entries;
    }
  }

  loadAccountEntitlements(directoryEntry) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/load_account_entitlements';
    let payload = {
      account: directoryEntry.toData()
    };

    return httpConnectionManager.send(apiPath, payload, true)
      .then(parseResponse)
      .catch(this.logErrorAndThrow);

    function parseResponse(response) {
      let entitlements = response;
      return entitlements;
    }
  }

  storeAccountEntitlements(directoryEntry, entitlements) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/store_account_entitlements';
    let payload = {
      account: directoryEntry.toData(),
      entitlements: entitlements
    };

    return httpConnectionManager.send(apiPath, payload, false)
      .catch(this.logErrorAndThrow);
  }

  loadManagedTradingGroups(directoryEntry) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/load_managed_trading_groups';
    let payload = {
      account: directoryEntry.toData()
    };

    return httpConnectionManager.send(apiPath, payload, true)
      .catch(this.logErrorAndThrow);
  }

  loadTradingGroup(directoryEntry) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/load_trading_group';
    let payload = {
      directory_entry: directoryEntry.toData()
    };

    return httpConnectionManager.send(apiPath, payload, true)
      .catch(this.logErrorAndThrow);
  }
}

export default Admin;
