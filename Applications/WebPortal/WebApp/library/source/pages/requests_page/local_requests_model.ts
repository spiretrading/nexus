import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { RequestsModel } from './requests_model';

type Status = Nexus.AccountModificationRequest.Status;
const Status = Nexus.AccountModificationRequest.Status;

/** Implements the RequestsModel using local memory. */
export class LocalRequestsModel extends RequestsModel {

  /** Constructs a LocalRequestsModel.
   *  @param account - The logged-in account's directory entry.
   *  @param entries - The initial request entries.
   *  @param details - The initial request details keyed by id.
   */
  constructor(account: Beam.DirectoryEntry,
      entries: RequestsModel.RequestEntry[],
      details: Map<number, RequestsModel.RequestDetail>) {
    super();
    this.account = account;
    this.entries = entries.slice();
    this.details = new Map(details);
  }

  /** Adds a request entry.
   *  @param entry - The entry to add.
   */
  public addEntry(entry: RequestsModel.RequestEntry): void {
    this.entries.push(entry);
  }

  /** Updates an existing entry's state and update time.
   *  @param id - The request id to update.
   *  @param update - The update containing the new status.
   */
  public updateEntry(id: number,
      update: Nexus.AccountModificationRequest.Update): void {
    const index = this.entries.findIndex(e => e.id === id);
    if(index >= 0) {
      this.entries[index] = {
        ...this.entries[index],
        state: update.status,
        updateTime: update.timestamp.toDate()
      };
    }
  }

  /** Adds a request detail.
   *  @param detail - The detail to add.
   */
  public addDetail(detail: RequestsModel.RequestDetail): void {
    this.details.set(detail.id, detail);
  }

  public async load(): Promise<void> {}

  public async loadRequestDirectory(submission: RequestsModel.Submission):
      Promise<RequestsModel.Response> {
    const filtered = this.filterEntries(this.entries, submission);
    const sorted = this.sortEntries(filtered, submission.filters.sortKey);
    const facetCounts = this.computeFacetCounts(filtered);
    const stateFiltered = sorted.filter(
      entry => matchesState(entry.state, submission.requestState));
    return {
      status: RequestsModel.ResponseStatus.READY,
      facetCounts,
      requestList: stateFiltered
    };
  }

  public async loadRequestDetail(id: number):
      Promise<RequestsModel.RequestDetail> {
    const detail = this.details.get(id);
    if(detail === undefined) {
      throw new Error(`Request detail not found: ${id}`);
    }
    return detail;
  }

  public async approve(id: number, effectiveDate: Beam.Date,
      comment: string): Promise<Nexus.AccountModificationRequest.Update> {
    return new Nexus.AccountModificationRequest.Update(
      Status.GRANTED, Beam.DirectoryEntry.INVALID, 0,
      Beam.DateTime.fromDate(new Date()));
  }

  public async reject(id: number, comment: string):
      Promise<Nexus.AccountModificationRequest.Update> {
    return new Nexus.AccountModificationRequest.Update(
      Status.REJECTED, Beam.DirectoryEntry.INVALID, 0,
      Beam.DateTime.fromDate(new Date()));
  }

  private filterEntries(entries: RequestsModel.RequestEntry[],
      submission: RequestsModel.Submission): RequestsModel.RequestEntry[] {
    let result = entries;
    if(submission.scope === RequestsModel.Scope.YOU) {
      result = result.filter(e => e.account.equals(this.account));
    }
    const filters = submission.filters;
    if(filters.categories.size > 0) {
      result = result.filter(e => filters.categories.has(e.category));
    }
    if(filters.query.length > 0) {
      const query = filters.query.toLowerCase();
      result = result.filter(e =>
        e.account.name.toLowerCase().includes(query) ||
        String(e.id).includes(query));
    }
    if(filters.startDate) {
      result = result.filter(
        e => filters.startDate.compare(Beam.Date.fromDate(e.updateTime)) <= 0);
    }
    if(filters.endDate) {
      result = result.filter(
        e => filters.endDate.compare(Beam.Date.fromDate(e.updateTime)) >= 0);
    }
    return result;
  }

  private sortEntries(entries: RequestsModel.RequestEntry[],
      sortKey: RequestsModel.SortField): RequestsModel.RequestEntry[] {
    const sorted = entries.slice();
    switch(sortKey) {
      case RequestsModel.SortField.LAST_UPDATED:
        sorted.sort((a, b) => b.updateTime.getTime() - a.updateTime.getTime());
        break;
      case RequestsModel.SortField.CREATED:
        sorted.sort(
          (a, b) => b.effectiveDate.getTime() - a.effectiveDate.getTime());
        break;
      case RequestsModel.SortField.ACCOUNT:
        sorted.sort((a, b) => a.account.name.localeCompare(b.account.name));
        break;
      case RequestsModel.SortField.EFFECTIVE_DATE:
        sorted.sort(
          (a, b) => b.effectiveDate.getTime() - a.effectiveDate.getTime());
        break;
      default:
        sorted.sort((a, b) => b.updateTime.getTime() - a.updateTime.getTime());
        break;
    }
    return sorted;
  }

  private computeFacetCounts(entries: RequestsModel.RequestEntry[]):
      RequestsModel.FacetCounts {
    let pending = 0;
    let approved = 0;
    let rejected = 0;
    for(const entry of entries) {
      if(matchesState(entry.state, RequestsModel.RequestState.PENDING)) {
        ++pending;
      }
      if(matchesState(entry.state, RequestsModel.RequestState.APPROVED)) {
        ++approved;
      }
      if(matchesState(entry.state, RequestsModel.RequestState.REJECTED)) {
        ++rejected;
      }
    }
    return {pending, approved, rejected};
  }

  private account: Beam.DirectoryEntry;
  private entries: RequestsModel.RequestEntry[];
  private details: Map<number, RequestsModel.RequestDetail>;
}

function matchesState(status: Status,
    requestState: RequestsModel.RequestState): boolean {
  switch(requestState) {
    case RequestsModel.RequestState.PENDING:
      return status === Status.PENDING || status === Status.REVIEWED ||
        status === Status.SCHEDULED;
    case RequestsModel.RequestState.APPROVED:
      return status === Status.GRANTED;
    case RequestsModel.RequestState.REJECTED:
      return status === Status.REJECTED;
  }
}
