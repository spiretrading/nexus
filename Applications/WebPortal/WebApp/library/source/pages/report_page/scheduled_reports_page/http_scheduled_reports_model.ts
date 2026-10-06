import * as Beam from 'beam';
import { formatReportDate } from '../generated_reports_page/report_date';
import { ScheduledReportsModel } from './scheduled_reports_model';

/** Retrieves scheduled reports and performs schedule actions through HTTP. */
export class HttpScheduledReportsModel extends ScheduledReportsModel {
  constructor() {
    super();
    this.loaded = false;
  }

  public async load(): Promise<void> {
    this.loaded = true;
  }

  public async query(submission: ScheduledReportsModel.Submission):
      Promise<ScheduledReportsModel.Response> {
    this.ensureLoaded();
    const response = await Beam.post(
      '/api/reporting_service/query_scheduled_reports',
      {filters: {query: submission.filters.query.trim()},
        page_index: submission.pageIndex});
    return {status: response.status, isEmpty: response.is_empty,
      filteredCount: response.filtered_count,
      schedules: (response.schedules ?? []).map(parseSchedule)};
  }

  public async run(id: string): Promise<void> {
    this.ensureLoaded();
    await Beam.post('/api/reporting_service/run_scheduled_report', {id});
  }

  public async duplicate(id: string): Promise<ScheduledReportsModel.Schedule> {
    this.ensureLoaded();
    return parseSchedule(await Beam.post(
      '/api/reporting_service/duplicate_scheduled_report', {id}));
  }

  public async delete(id: string): Promise<void> {
    this.ensureLoaded();
    await Beam.post('/api/reporting_service/delete_scheduled_report', {id});
  }

  private ensureLoaded(): void {
    if(!this.loaded) {
      throw new Error('Model not loaded.');
    }
  }

  private loaded: boolean;
}

function parseSchedule(value: any): ScheduledReportsModel.Schedule {
  const date = Beam.Date.fromJson(value.run_date);
  const text = date.toJson();
  return {id: value.id, type: value.type,
    parameters: value.parameters.map((parameter: any) =>
      ({label: parameter.label, value: parameter.value})),
    repeats: value.repeats, runDate: {
      value: `${text.slice(0, 4)}-${text.slice(4, 6)}-${text.slice(6)}`,
      label: formatReportDate(date)}};
}
