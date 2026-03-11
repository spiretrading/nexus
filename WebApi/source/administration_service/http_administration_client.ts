import * as Beam from 'beam';
import { RiskParameters } from '..';
import { AccountIdentity } from './account_identity';
import { AccountModificationRequest } from './account_modification_request';
import { AccountRoles } from './account_roles';
import { AdministrationClient } from './administration_client';
import { EntitlementModification } from './entitlement_modification';
import { Message } from './message';
import { RiskModification } from './risk_modification';
import { TradingGroup } from './trading_group';

/** Implements the AdministrationClient using HTTP requests. */
export class HttpAdministrationClient extends AdministrationClient {
  public async loadAccountsByRoles(roles: AccountRoles):
      Promise<Beam.DirectoryEntry[]> {
    const response = await Beam.post(
      '/api/administration_service/load_accounts_by_roles',
      {
        roles: roles.toJson()
      });
    return Beam.arrayFromJson(Beam.DirectoryEntry, response);
  }

  public async loadAdministratorsRootEntry(): Promise<Beam.DirectoryEntry> {
    const response = await Beam.post(
      '/api/administration_service/load_administrators_root_entry', {});
    return Beam.DirectoryEntry.fromJson(response);
  }

  public async loadServicesRootEntry(): Promise<Beam.DirectoryEntry> {
    const response = await Beam.post(
      '/api/administration_service/load_services_root_entry', {});
    return Beam.DirectoryEntry.fromJson(response);
  }

  public async loadTradingGroupsRootEntry(): Promise<Beam.DirectoryEntry> {
    const response = await Beam.post(
      '/api/administration_service/load_trading_groups_root_entry', {});
    return Beam.DirectoryEntry.fromJson(response);
  }

  public async searchAccounts(prefix: string):
      Promise<[Beam.DirectoryEntry, Beam.DirectoryEntry, AccountRoles][]> {
    const response = await Beam.post(
      '/api/service_locator/search_directory_entry',
      {
        name: prefix
      });
    const matches: [Beam.DirectoryEntry, Beam.DirectoryEntry, AccountRoles][] =
      [];
    for(const result of response) {
      matches.push([Beam.DirectoryEntry.fromJson(result.group),
        Beam.DirectoryEntry.fromJson(result.directory_entry),
        AccountRoles.fromJson(result.roles)]);
    }
    return matches;
  }

  public async loadTradingGroup(directoryEntry: Beam.DirectoryEntry):
      Promise<TradingGroup> {
    const response = await Beam.post(
      '/api/administration_service/load_trading_group',
      {
        directory_entry: directoryEntry.toJson()
      });
    return TradingGroup.fromJson(response);
  }

  public async loadManagedTradingGroups(account: Beam.DirectoryEntry):
      Promise<Beam.DirectoryEntry[]> {
    const response = await Beam.post(
      '/api/administration_service/load_managed_trading_groups',
      {
        account: account.toJson()
      });
    return Beam.arrayFromJson(Beam.DirectoryEntry, response);
  }

  public async loadParentTradingGroup(account: Beam.DirectoryEntry):
      Promise<Beam.DirectoryEntry> {
    const response = await Beam.post(
      '/api/administration_service/load_parent_trading_group',
      {
        account: account.toJson()
      });
    return Beam.DirectoryEntry.fromJson(response);
  }

  public async loadAccountRoles(account: Beam.DirectoryEntry):
      Promise<AccountRoles> {
    const response = await Beam.post(
      '/api/administration_service/load_account_roles',
      {
        account: account.toJson()
      });
    return AccountRoles.fromJson(response);
  }

  public async storeAccountRoles(account: Beam.DirectoryEntry,
      roles: AccountRoles): Promise<AccountRoles> {
    const response = await Beam.post(
      '/api/administration_service/store_account_roles',
      {
        account: account.toJson(),
        roles: roles.toJson()
      });
    return AccountRoles.fromJson(response);
  }

  public async loadAccountIdentity(account: Beam.DirectoryEntry):
      Promise<AccountIdentity> {
    const response = await Beam.post(
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
        identity: identity.toJson()
      });
  }

  public async loadAccountEntitlements(account: Beam.DirectoryEntry):
      Promise<Beam.Set<Beam.DirectoryEntry>> {
    const response = await Beam.post(
      '/api/administration_service/load_account_entitlements',
      {
        account: account.toJson()
      });
    return Beam.Set.fromJson(Beam.DirectoryEntry, response);
  }

  public async loadEntitlementModification(id: number):
      Promise<EntitlementModification> {
    const response = await Beam.post(
      '/api/administration_service/load_entitlement_modification',
      {
        id: id
      });
    return EntitlementModification.fromJson(response);
  }

  public async submitEntitlementModificationRequest(
      account: Beam.DirectoryEntry, modification: EntitlementModification,
      comment: Message, effectiveDate: Beam.Date):
      Promise<AccountModificationRequest> {
    const response = await Beam.post(
      '/api/administration_service/submit_entitlement_modification_request',
      {
        account: account.toJson(),
        modification: modification.toJson(),
        comment: comment.toJson(),
        effective_date: effectiveDate.toJson()
      });
    return AccountModificationRequest.fromJson(response);
  }

  public async loadRiskParameters(account: Beam.DirectoryEntry):
      Promise<RiskParameters> {
    const response = await Beam.post(
      '/api/administration_service/load_risk_parameters',
      {
        account: account.toJson()
      });
    return RiskParameters.fromJson(response);
  }

  public async loadRiskModification(id: number): Promise<RiskModification> {
    const response = await Beam.post(
      '/api/administration_service/load_risk_modification',
      {
        id: id
      });
    return RiskModification.fromJson(response);
  }

  public async submitRiskModificationRequest(account: Beam.DirectoryEntry,
      modification: RiskModification, comment: Message,
      effectiveDate: Beam.Date): Promise<AccountModificationRequest> {
    const response = await Beam.post(
      '/api/administration_service/submit_risk_modification_request',
      {
        account: account.toJson(),
        modification: modification.toJson(),
        comment: comment.toJson(),
        effective_date: effectiveDate.toJson()
      });
    return AccountModificationRequest.fromJson(response);
  }

  public async loadAccountModificationRequest(id: number):
      Promise<AccountModificationRequest> {
    const response = await Beam.post(
      '/api/administration_service/load_account_modification_request',
      {
        id: id
      });
    return AccountModificationRequest.fromJson(response);
  }

  public async loadAccountModificationRequestIds(
      account: Beam.DirectoryEntry, startId: number, maxCount: number):
      Promise<number[]> {
    const response = await Beam.post(
      '/api/administration_service/load_account_modification_request_ids',
      {
        account: account.toJson(),
        start_id: startId,
        max_count: maxCount
      });
    return response;
  }

  public async loadManagedAccountModificationRequestIds(
      account: Beam.DirectoryEntry, startId: number, maxCount: number):
      Promise<number[]> {
    const response = await Beam.post('/api/administration_service/' +
      'load_managed_account_modification_request_ids',
      {
        account: account.toJson(),
        start_id: startId,
        max_count: maxCount
      });
    return response;
  }

  public async loadAccountModificationRequestStatus(id: number):
      Promise<AccountModificationRequest.Update> {
    const response = await Beam.post(
      '/api/administration_service/load_account_modification_request_status',
      {
        id: id
      });
    return AccountModificationRequest.Update.fromJson(response);
  }

  public async loadAccountModificationRequestUpdates(id: number):
      Promise<AccountModificationRequest.Update[]> {
    const response = await Beam.post(
      '/api/administration_service/load_account_modification_request_updates',
      {
        id: id
      });
    return response.map(AccountModificationRequest.Update.fromJson);
  }

  public async approveAccountModificationRequest(id: number,
      comment: Message, effectiveDate: Beam.Date):
      Promise<AccountModificationRequest.Update> {
    const response = await Beam.post(
      '/api/administration_service/approve_account_modification_request',
      {
        id: id,
        comment: comment.toJson(),
        effective_date: effectiveDate.toJson()
      });
    return AccountModificationRequest.Update.fromJson(response);
  }

  public async rejectAccountModificationRequest(id: number,
      comment: Message): Promise<AccountModificationRequest.Update> {
    const response = await Beam.post(
      '/api/administration_service/reject_account_modification_request',
      {
        id: id,
        comment: comment.toJson()
      });
    return AccountModificationRequest.Update.fromJson(response);
  }

  public async loadMessage(id: number): Promise<Message> {
    const response = await Beam.post(
      '/api/administration_service/load_message',
      {
        id: id
      });
    return Message.fromJson(response);
  }

  public async loadMessageIds(id: number): Promise<number[]> {
    const response = await Beam.post(
      '/api/administration_service/load_message_ids',
      {
        id: id
      });
    return response;
  }

  public async sendAccountModificationRequestMessage(id: number,
      message: Message): Promise<Message> {
    const response = await Beam.post('/api/administration_service/' +
      'send_account_modification_request_message',
      {
        id: id,
        message: message.toJson()
      });
    return Message.fromJson(response);
  }

  public async createGroup(name: string): Promise<Beam.DirectoryEntry> {
    const response = await Beam.post('/api/service_locator/create_group',
      {
        name: name
      });
    return Beam.DirectoryEntry.fromJson(response);
  }

  public async createAccount(name: string, group: Beam.DirectoryEntry,
      identity: AccountIdentity, roles: AccountRoles):
      Promise<Beam.DirectoryEntry> {
    const response = await Beam.post('/api/service_locator/create_account',
      {
        name: name,
        group: group.toJson(),
        identity: identity.toJson(),
        roles: roles.toJson()
      });
    return Beam.DirectoryEntry.fromJson(response);
  }

  public async open(): Promise<void> {
    return;
  }

  public async close(): Promise<void> {
    return;
  }
}
