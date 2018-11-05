import * as Beam from 'beam';
import {RiskParameters} from '..';
import {AccountModificationRequest, AccountRoles, AdministrationClient, Message,
  RiskModification} from '.';

/** Implements the AdministrationClient using web services. */
export class WebAdministrationClient extends AdministrationClient {

  /** Constructs a WebAdministrationClient. */
  constructor() {
    super();
  }

  public async loadAccountRoles(account: Beam.DirectoryEntry):
      Promise<AccountRoles> {
    try {
      let response = await Beam.post(
        '/api/administration_service/load_account_roles',
        {
          account: account.toJson()
        });
      return AccountRoles.fromJson(response);
    } catch(e) {
      throw new Beam.ServiceError(e.toString());
    }
  }

  public async loadRiskParameters(account: Beam.DirectoryEntry):
      Promise<RiskParameters> {
    try {
      let response = await Beam.post(
        '/api/administration_service/load_risk_parameters',
        {
          account: account.toJson()
        });
      return RiskParameters.fromJson(response);
    } catch(e) {
      throw new Beam.ServiceError(e.toString());
    }
  }

  public async loadRiskModification(id: number): Promise<RiskModification> {
    try {
      let response = await Beam.post(
        '/api/administration_service/load_risk_modification',
        {
          id: id
        });
      return RiskModification.fromJson(response);
    } catch(e) {
      throw new Beam.ServiceError(e.toString());
    }
  }

  public async submitRiskModificationRequest(account: Beam.DirectoryEntry,
      modification: RiskModification, comment: Message):
      Promise<AccountModificationRequest> {
    try {
      let response = await Beam.post(
        '/api/administration_service/submit_risk_modification_request',
        {
          account: account.toJson(),
          modification: modification.toJson(),
          comment: comment.toJson()
        });
      return AccountModificationRequest.fromJson(response);
    } catch(e) {
      throw new Beam.ServiceError(e.toString());
    }
  }

  public async open(): Promise<void> {
    return;
  }

  public async close(): Promise<void> {
    return;
  }
}
