import * as Beam from 'beam';
import { SortableTableHeaderCell } from '../../../components';
import { ActivityTable } from './activity_table';
import { ReportActivityModel } from './report_activity_model';
import { ReportActivityStatusTag } from './report_activity_status_tag';

/** Stores report jobs locally for report activity and testing. */
export class LocalReportActivityModel extends ReportActivityModel {

  /** Constructs a local model.
   * @param activities - The initial report jobs, in default order.
   */
  constructor(activities: readonly ActivityTable.Activity[]) {
    super();
    this.loaded = false;
    this.entries = new Map(activities.map(entry => [entry.id, copy(entry)]));
  }

  /** Returns whether this model has been loaded. */
  public get isLoaded(): boolean {
    return this.loaded;
  }

  /** Returns copies of the report jobs in default order. */
  public get activities(): readonly ActivityTable.Activity[] {
    this.ensureLoaded();
    return Array.from(this.entries.values(), copy);
  }

  public async load(): Promise<void> {
    this.loaded = true;
  }

  public async loadActivities(submission: ReportActivityModel.Submission):
      Promise<ReportActivityModel.Response> {
    this.ensureLoaded();
    const entries = Array.from(this.entries.values());
    if(submission.sort.order !== SortableTableHeaderCell.SortOrder.NONE) {
      entries.sort((left, right) => {
        const result = (() => {
          switch(submission.sort.column) {
            case ActivityTable.Column.TYPE:
              return left.type.localeCompare(right.type);
            case ActivityTable.Column.PARAMETERS:
              return left.parameters.join(' \u2022 ').localeCompare(
                right.parameters.join(' \u2022 '));
            case ActivityTable.Column.STATUS:
              return left.status - right.status;
            case ActivityTable.Column.DATE_MODIFIED:
              return left.dateModified.compare(right.dateModified);
          }
        })();
        if(submission.sort.order ===
            SortableTableHeaderCell.SortOrder.DESCENDING) {
          return -result;
        }
        return result;
      });
    }
    const start = submission.pageIndex * ReportActivityModel.PAGE_SIZE;
    return {status: ReportActivityModel.ResponseStatus.READY,
      isEmpty: entries.length === 0, totalCount: entries.length,
      activities: entries.slice(start, start + ReportActivityModel.PAGE_SIZE).
        map(copy)};
  }

  public async cancel(ids: readonly string[]): Promise<void> {
    this.ensureLoaded();
    for(const id of ids) {
      this.entries.delete(id);
    }
  }

  public async retry(ids: readonly string[]): Promise<void> {
    this.ensureLoaded();
    for(const id of ids) {
      const entry = this.entries.get(id);
      if(entry?.status === ReportActivityStatusTag.Status.FAILED) {
        this.entries.set(id, {...entry,
          status: ReportActivityStatusTag.Status.GENERATING,
          dateModified: Beam.Date.today()});
      }
    }
  }

  private ensureLoaded(): void {
    if(!this.isLoaded) {
      throw new Error('Model not loaded.');
    }
  }

  private loaded: boolean;
  private entries: Map<string, ActivityTable.Activity>;
}

function copy(entry: ActivityTable.Activity): ActivityTable.Activity {
  return {...entry, parameters: [...entry.parameters],
    dateModified: new Beam.Date(entry.dateModified.year,
      entry.dateModified.month, entry.dateModified.day)};
}
