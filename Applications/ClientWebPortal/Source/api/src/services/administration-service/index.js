import httpConnectionManager from '../commons/http-connection-manager';
import AccountRoles from '../commons/account-roles';
import AccountIdentity from './account-identity';
import RiskParameters from '../risk-service/risk-parameters';
import DirectoryEntry from '../../definitions/directory-entry';

/** Spire admin client class */
class Admin {
  constructor() {
    this.loadManagedTradingGroups = this.loadManagedTradingGroups.bind(this);
    this.loadTradingGroup = this.loadTradingGroup.bind(this);
    this.loadAccountRoles = this.loadAccountRoles.bind(this);
    this.loadRiskParameters = this.loadRiskParameters.bind(this);
    this.loadAccountIdentity = this.loadAccountIdentity.bind(this);
    this.storeAccountIdentity = this.storeAccountIdentity.bind(this);
    this.loadAccountEntitlements = this.loadAccountEntitlements.bind(this);
    this.storeAccountEntitlements = this.storeAccountEntitlements.bind(this);
    this.storeRiskParameters = this.storeRiskParameters.bind(this);
  }

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
      let accRoles = AccountRoles.parse(roles)
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
      console.debug(response);
      console.debug(AccountIdentity.fromData(response));
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
      return RiskParameters.fromData(response);
    }
  }

  storeRiskParameters(directoryEntry, riskParameters) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/store_risk_parameters';
    let payload = {
      account: directoryEntry.toData(),
      risk_parameters: riskParameters.toData()
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
      .then(response => {
        return response.map(value => {
          return DirectoryEntry.fromData(value);
        });
      })
      .catch(this.logErrorAndThrow);
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

  storeAccountRoles(directoryEntry, roles) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/store_account_roles';
    let payload = {
      account: directoryEntry.toData(),
      roles: AccountRoles.encode(roles)
    };

    return httpConnectionManager.send(apiPath, payload, true)
      .catch(this.logErrorAndThrow);
  }

  submitEntitlementModificationRequest(directoryEntry, entitlementModification, comment) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/submit_entitlement_modification_request';
    let payload = {
      account: directoryEntry.toData(),
      modification: entitlementModification.toData(),
      comment: comment
    };

    // return httpConnectionManager.send(apiPath, payload, true)
    //   .catch(this.logErrorAndThrow);

    return new Promise((resolve, reject) => {
      resolve('EN0042');
    });
  }
}

export default Admin;
