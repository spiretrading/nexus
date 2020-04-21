import * as Beam from 'beam';
import { RiskParameters } from '..';
import { AccountIdentity } from './account_identity';
import { AccountModificationRequest } from './account_modification_request';
import { AccountRoles } from './account_roles';
import { AdministrationClient } from './administration_client';
import { EntitlementModification } from './entitlement_modification';
import { Message } from './message';
import { RiskModification } from './risk_modification';

/** Implements the AdministrationClient using HTTP requests. */
export class HttpAdministrationClient extends AdministrationClient {
  public async loadAccountRoles(account: Beam.DirectoryEntry):
      Promise<AccountRoles> {
    let response = await Beam.post(
      '/api/administration_service/load_account_roles',
      {
        account: account.toJson()
      });
    return AccountRoles.fromJson(response);
  }

  public async storeAccountRoles(account: Beam.DirectoryEntry,
      roles: AccountRoles): Promise<AccountRoles> {
    let response = await Beam.post(
      '/api/administration_service/store_account_roles',
      {
        account: account.toJson(),
        roles: roles.toJson()
      });
    return AccountRoles.fromJson(response);
  }

  public async loadAccountIdentity(account: Beam.DirectoryEntry):
      Promise<AccountIdentity> {
    let response = await Beam.post(
      '/api/administration_service/load_account_identity',
      {
        account: account.toJson()
      });
    return AccountIdentity.fromJson(response);
  }

  public async storeAccountIdentity(account: Beam.DirectoryEntry,
      identity: AccountIdentity): Promise<void> {
    await Beam.post('/api/administration_service/store_account_identity',
      {
        account: account.toJson(),
        roles: identity.toJson()
      });
  }

  public async loadAccountEntitlements(account: Beam.DirectoryEntry):
      Promise<Beam.Set<Beam.DirectoryEntry>> {
    let response = await Beam.post(
      '/api/administration_service/load_account_entitlements',
      {
        account: account.toJson()
      });
    return Beam.Set.fromJson(Beam.DirectoryEntry, response);
  }

  public async loadEntitlementModification(id: number):
      Promise<EntitlementModification> {
    let response = await Beam.post(
      '/api/administration_service/load_entitlement_modification',
      {
        id: id
      });
    return EntitlementModification.fromJson(response);
  }

  public async submitEntitlementModificationRequest(
      account: Beam.DirectoryEntry, modification: EntitlementModification,
      comment: Message): Promise<AccountModificationRequest> {
    let response = await Beam.post(
      '/api/administration_service/submit_entitlement_modification_request',
      {
        account: account.toJson(),
        modification: modification.toJson(),
        comment: comment.toJson()
      });
    return AccountModificationRequest.fromJson(response);
  }

  public async loadRiskParameters(account: Beam.DirectoryEntry):
      Promise<RiskParameters> {
    let response = await Beam.post(
      '/api/administration_service/load_risk_parameters',
      {
        account: account.toJson()
      });
    return RiskParameters.fromJson(response);
  }

  public async loadRiskModification(id: number): Promise<RiskModification> {
    let response = await Beam.post(
      '/api/administration_service/load_risk_modification',
      {
        id: id
      });
    return RiskModification.fromJson(response);
  }

  public async submitRiskModificationRequest(account: Beam.DirectoryEntry,
      modification: RiskModification, comment: Message):
      Promise<AccountModificationRequest> {
    let response = await Beam.post(
      '/api/administration_service/submit_risk_modification_request',
      {
        account: account.toJson(),
        modification: modification.toJson(),
        comment: comment.toJson()
      });
    return AccountModificationRequest.fromJson(response);
  }

  public async open(): Promise<void> {
    return;
  }

  public async close(): Promise<void> {
    return;
  }
}
