import httpConnectionManager from '../commons/http-connection-manager';

/** Spire compliance service client class */
class ComplianceServiceClient {
  /** @private */
  logErrorAndThrow(xhr) {
    let errorMessage = 'Spire Compliance Service Client: Unexpected error happened.';
    console.error(errorMessage);
    console.error(xhr);
    throw errorMessage;
  }

  loadComplianceRuleEntries(directoryEntry) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'compliance_service/load_directory_entry_compliance_rule_entry';
    let payload = {
      directory_entry: directoryEntry
    };

    return httpConnectionManager.send(apiPath, payload, true)
      .catch(this.logErrorAndThrow);
  }
}

export default new ComplianceServiceClient();
