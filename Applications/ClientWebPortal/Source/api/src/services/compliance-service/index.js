import httpConnectionManager from '../commons/http-connection-manager';
import Money from '../../definitions/money';
import DataType from '../../definitions/data-type';
import dataTypeConverter from '../commons/data-type-converter';

/** Spire compliance service client class */
class ComplianceService {
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
      directory_entry: directoryEntry.toData()
    };

    return httpConnectionManager.send(apiPath, payload, true)
      .then(ruleEntries => {
        for (let i=0; i<ruleEntries.length; i++) {
          let parameters = ruleEntries[i].schema.parameters;
          for (let j=0; j<parameters.length; j++) {
            let parameterType = parameters[j].value.which;
            if (parameterType == DataType.MONEY) {
              parameters[j].value.value = Money.fromRepresentation(parameters[j].value.value);
            }
          }
        }
        return ruleEntries;
      })
      .catch(this.logErrorAndThrow);
  }

  addComplianceRuleEntry(directoryEntry, state, schema) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'compliance_service/add_compliance_rule_entry';
    let payload = {
      directory_entry: directoryEntry.toData(),
      state: state,
      schema: schema
    };
    dataTypeConverter.toData(payload);
    return httpConnectionManager.send(apiPath, payload, false)
      .catch(this.logErrorAndThrow);
  }

  updateComplianceRuleEntry(ruleEntry) {
    dataTypeConverter.toData.apply(dataTypeConverter, [ruleEntry.schema]);
    let apiPath = Config.BACKEND_API_ROOT_URL + 'compliance_service/update_compliance_rule_entry';
    let payload = {
      rule_entry: ruleEntry
    };

    return httpConnectionManager.send(apiPath, payload, false)
      .catch(this.logErrorAndThrow);
  }

  deleteComplianceRuleEntry(id) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'compliance_service/delete_compliance_rule_entry';
    let payload = {
      id: id
    };

    return httpConnectionManager.send(apiPath, payload, false)
      .catch(this.logErrorAndThrow);
  }
}

export default ComplianceService;
