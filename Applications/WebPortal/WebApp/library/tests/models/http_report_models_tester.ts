import * as assert from 'node:assert/strict';
import { afterEach, beforeEach, describe, it } from 'node:test';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { LocalAccountGroupQueryModel, SortableTableHeaderCell } from
  '../../source/components';
import { DateRange, Interval, LocalTickerQueryModel, ScopeQueryModel } from
  '../../source/models';
import { ActivityTable, GeneratedReportsModel, HttpCreateReportModel,
  HttpEditScheduledReportModel, HttpGeneratedReportsModel,
  HttpReportActivityModel, HttpReportDetailModel, HttpReportModel,
  HttpScheduledReportsModel, ReportActivityModel, ReportActivityStatusTag,
  ReportFormTemplate, ReportTable } from '../../source/pages/report_page';

interface Call {
  url: string;
  body: any;
}

interface Reply {
  status: number;
  body?: any;
}

class Request {
  public static calls: Call[] = [];
  public static handler: (call: Call) => Reply | Promise<Reply>;
  public status: number;
  public statusText = 'Service failed';
  public responseText: string;
  public onload: () => void;

  public open(method: string, url: string): void {
    assert.equal(method, 'POST');
    this.url = url;
  }

  public setRequestHeader(): void {}

  public send(body: string): void {
    const call = {url: this.url, body: JSON.parse(body)};
    Request.calls.push(call);
    Promise.resolve(Request.handler(call)).then(reply => {
      this.status = reply.status;
      this.responseText = JSON.stringify(reply.body) ?? '';
      this.onload();
    });
  }

  private url: string;
}

const DEFINITION = {id: 'example', name: 'Example', parameters: [
  {name: 'account', label: 'Account', type: 'DirectoryEntry', required: true},
  {name: 'accounts', label: 'Accounts', type: 'DirectoryEntryList'},
  {name: 'range', label: 'Range', type: 'DateRange'},
  {name: 'currency', label: 'Currency', type: 'Currency'},
  {name: 'money', label: 'Money', type: 'Money'},
  {name: 'scope', label: 'Scope', type: 'Scope'},
  {name: 'count', label: 'Count', type: 'Integer', default: 0},
  {name: 'optional', label: 'Optional', type: 'Decimal'}],
  output: {media_type: 'text/csv', extension: 'csv'}};

function makeValue(): ReportFormTemplate.Value {
  return {reportType: 'example', parameters: {
    account: Beam.DirectoryEntry.STAR_DIRECTORY,
    accounts: [Beam.DirectoryEntry.makeAccount(1, 'Alice')],
    range: new DateRange(new Beam.Date(2026, 10, 1), null),
    currency: Nexus.Currency.fromJson(840), money: Nexus.Money.parse('12.34'),
    scope: Nexus.Scope.GLOBAL, count: 0, optional: null},
    recipients: [Beam.DirectoryEntry.makeDirectory(2, 'Group')],
    scheduled: false, scheduleDateTime: null, repeats: false,
    repeatInterval: null};
}

function lookups(): [LocalAccountGroupQueryModel, ScopeQueryModel] {
  return [new LocalAccountGroupQueryModel([]),
    new ScopeQueryModel(new LocalTickerQueryModel([]))];
}

describe('HTTP report models', () => {
  const original = globalThis.XMLHttpRequest;
  beforeEach(() => {
    Request.calls = [];
    Request.handler = () => ({status: 200});
    globalThis.XMLHttpRequest = Request as unknown as typeof XMLHttpRequest;
  });
  afterEach(() => { globalThis.XMLHttpRequest = original; });

  it('requires_load_for_every_subpage', async () => {
    const [accounts, scopes] = lookups();
    const create = new HttpCreateReportModel(accounts, scopes);
    const edit = new HttpEditScheduledReportModel(accounts, scopes);
    const activity = new HttpReportActivityModel();
    const generated = new HttpGeneratedReportsModel(accounts);
    const schedules = new HttpScheduledReportsModel();
    const details = new HttpReportDetailModel();
    assert.throws(() => create.reports, /not loaded/);
    assert.throws(() => edit.accountModel, /not loaded/);
    assert.throws(() => generated.recipientModel, /not loaded/);
    await assert.rejects(create.submit(makeValue()), /not loaded/);
    await assert.rejects(edit.loadReport('id'), /not loaded/);
    await assert.rejects(edit.submit('id', makeValue()), /not loaded/);
    await assert.rejects(activity.cancel(['id']), /not loaded/);
    await assert.rejects(activity.retry(['id']), /not loaded/);
    await assert.rejects(generated.delete(['id']), /not loaded/);
    await assert.rejects(generated.share(['id'], []), /not loaded/);
    await assert.rejects(generated.download(['id']), /not loaded/);
    await assert.rejects(schedules.run('id'), /not loaded/);
    await assert.rejects(schedules.duplicate('id'), /not loaded/);
    await assert.rejects(schedules.delete('id'), /not loaded/);
    await assert.rejects(details.loadReport('id'), /not loaded/);
    assert.equal(Request.calls.length, 0);
  });

  it('loads_live_definitions_and_submits_typed_values', async () => {
    const [accounts, scopes] = lookups();
    const model = new HttpCreateReportModel(accounts, scopes);
    Request.handler = call => {
      if(call.url.endsWith('load_report_definitions')) {
        return {status: 200, body: [DEFINITION]};
      }
      return {status: 200, body: 'job-id'};
    };
    await Promise.all([model.load(), model.load()]);
    assert.equal(Request.calls.length, 1);
    assert.equal(model.reports[0].parameters[6].defaultValue, 0);
    assert.equal(model.accountModel, accounts);
    assert.equal(model.scopeModel, scopes);
    assert.equal(await model.submit(makeValue()), 'job-id');
    const call = Request.calls.at(-1);
    assert.equal(call.url, '/api/reporting_service/submit_report');
    assert.deepEqual(call.body, {report_type: 'example',
      time_zone: Intl.DateTimeFormat().resolvedOptions().timeZone, parameters: {
      account: Beam.DirectoryEntry.STAR_DIRECTORY.toJson(),
      accounts: [Beam.DirectoryEntry.makeAccount(1, 'Alice').toJson()],
      range: {start: '20261001', end: null,
        rules: {start: {type: 'SpecificDate', value: {date: '20261001'}},
          end: null}}, currency: 840,
      money: Nexus.Money.parse('12.34').toJson(),
      scope: Nexus.Scope.GLOBAL.toJson(), count: 0, optional: null},
      recipients: [Beam.DirectoryEntry.makeDirectory(2, 'Group').toJson()],
      scheduled: false, schedule_date_time: null, repeats: false,
      repeat_interval: null});
    await model.load();
    assert.equal(Request.calls.filter(call =>
      call.url.endsWith('load_report_definitions')).length, 2);
  });

  it('recovers_catalog_loading_and_propagates_server_errors', async () => {
    const model = new HttpCreateReportModel(...lookups());
    Request.handler = () => ({status: 503, body: {error: 'Unavailable'}});
    await assert.rejects(model.load(), (error: any) => error.code === 503);
    assert.throws(() => model.reports, /not loaded/);
    Request.handler = () => ({status: 200, body: [DEFINITION]});
    await model.load();
    Request.handler = () => ({status: 501, body: {error: 'Not implemented'}});
    await assert.rejects(model.submit(makeValue()), (error: any) =>
      error.code === 501 && error.message === 'Not implemented');
  });

  it('maps_activity_dates_and_actions', async () => {
    const model = new HttpReportActivityModel();
    await model.load();
    Request.handler = call => {
      if(call.url.endsWith('query_report_activities')) {
        return {status: 200, body: {status: 1, is_empty: false, total_count: 51,
          activities: [{id: 'job', type: 'Example', parameters: ['USD'],
            status: 0, date_modified: '20261005T123456.125'}]}};
      }
      return {status: 200};
    };
    const response = await model.query({
      sort: {column: ActivityTable.Column.DATE_MODIFIED,
        order: SortableTableHeaderCell.SortOrder.DESCENDING}, pageIndex: 1});
    assert.deepEqual(Request.calls[0].body,
      {sort: {column: 3, order: 2}, page_index: 1});
    assert.equal(response.status, ReportActivityModel.ResponseStatus.READY);
    assert.equal(response.totalCount, 51);
    assert.equal(
      response.activities[0].dateModified.toJson(), '20261005T123456.125');
    assert.equal(response.activities[0].status,
      ReportActivityStatusTag.Status.GENERATING);
    await model.cancel(['job']);
    await model.retry(['job']);
    assert.ok(Request.calls[1].url.endsWith('cancel_report_jobs'));
    assert.ok(Request.calls[2].url.endsWith('retry_report_jobs'));
    assert.deepEqual(Request.calls[2].body, {ids: ['job']});
  });

  it('maps_generated_filters_results_and_mutations', async () => {
    const model = new HttpGeneratedReportsModel(lookups()[0]);
    await model.load();
    Request.handler = call => {
      if(call.url.endsWith('query_generated_reports')) {
        return {status: 200, body: {status: 1, is_empty: false,
          filtered_count: 1, reports: [{id: 'job', type: 'Example',
            parameters: ['USD'], url: '/reports/job',
            date_created: '20261005T235959.125'}]}};
      }
      return {status: 200};
    };
    const response = await model.query({filters: {query: ' USD ',
      dateRange: new DateRange(null, new Beam.Date(2026, 10, 5))},
      sort: {column: ReportTable.Column.TYPE,
        order: SortableTableHeaderCell.SortOrder.ASCENDING}, pageIndex: 0});
    assert.deepEqual(Request.calls[0].body, {filters: {query: 'USD',
      start_date: null, end_date: '20261005'},
      sort: {column: 0, order: 1}, page_index: 0});
    assert.equal(response.status, GeneratedReportsModel.ResponseStatus.READY);
    assert.equal(response.filteredCount, 1);
    assert.equal(
      response.reports[0].dateCreated.toJson(), '20261005T235959.125');
    const recipient = Beam.DirectoryEntry.makeAccount(2, 'Bob');
    await model.share(['job'], [recipient]);
    await model.delete(['job']);
    assert.deepEqual(Request.calls[1].body,
      {ids: ['job'], recipients: [recipient.toJson()]});
    assert.ok(Request.calls[1].url.endsWith('share_reports'));
    assert.ok(Request.calls[2].url.endsWith('delete_reports'));
  });

  it('maps_schedules_and_editable_values', async () => {
    const listing = new HttpScheduledReportsModel();
    const edit = new HttpEditScheduledReportModel(...lookups());
    const saved = {report_type: 'example', parameters: {
      account: Beam.DirectoryEntry.STAR_DIRECTORY.toJson(), count: 0},
      recipients: [] as unknown[], scheduled: true,
      schedule_date_time: '20261101T123000',
      repeats: true, repeat_interval: {count: 2, unit: Interval.Unit.MONTH}};
    const schedule = {id: 'schedule', type: 'Example',
      parameters: [{label: 'Account', value: '*'}],
      repeats: true, run_date: '20261101'};
    Request.handler = call => {
      if(call.url.endsWith('load_report_definitions')) {
        return {status: 200, body: [DEFINITION]};
      } else if(call.url.endsWith('query_scheduled_reports')) {
        return {status: 200, body: {status: 1, is_empty: false,
          filtered_count: 1, schedules: [schedule]}};
      } else if(call.url.endsWith('load_scheduled_report')) {
        assert.equal(call.body.time_zone,
          Intl.DateTimeFormat().resolvedOptions().timeZone);
        return {status: 200, body: saved};
      } else if(call.url.endsWith('duplicate_scheduled_report')) {
        return {status: 200, body: {...schedule, id: 'copy'}};
      }
      return {status: 200};
    };
    await listing.load();
    await edit.load();
    const page = await listing.query({filters: {query: ' * '}, pageIndex: 2});
    assert.deepEqual(Request.calls.at(-1).body,
      {filters: {query: '*'}, page_index: 2});
    assert.equal(page.schedules[0].runDate.value, '2026-11-01');
    assert.equal(page.schedules[0].runDate.label, 'Nov 01, 2026');
    assert.equal((await listing.duplicate('schedule')).id, 'copy');
    await listing.run('schedule');
    await listing.delete('schedule');
    const value = await edit.loadReport('schedule');
    assert.equal(value.scheduleDateTime.toJson(), '20261101T123000');
    assert.equal(value.repeatInterval.unit, Interval.Unit.MONTH);
    assert.ok((value.parameters.account as Beam.DirectoryEntry).
      equals(Beam.DirectoryEntry.STAR_DIRECTORY));
    await edit.submit('schedule', value);
    assert.deepEqual(Request.calls.at(-1).body,
      {id: 'schedule', ...saved,
        time_zone: Intl.DateTimeFormat().resolvedOptions().timeZone,
        parameters: {
        account: Beam.DirectoryEntry.STAR_DIRECTORY.toJson(), accounts: null,
        range: null, currency: null, money: null, scope: null,
        count: 0, optional: null}});
  });

  it('maps_detail_content_and_download_url', async () => {
    const model = new HttpReportDetailModel();
    await model.load();
    Request.handler = () => ({status: 200, body: {id: 'job', title: 'Example',
      parameters: [{label: 'Currency', value: 'USD'}],
      file_path: '/api/reporting_service/download_report?id=job',
      content: 'a,b'}});
    const report = await model.loadReport('job');
    assert.equal(report.content, 'a,b');
    assert.equal(report.filePath,
      '/api/reporting_service/download_report?id=job');
    assert.deepEqual(Request.calls[0].body, {id: 'job'});
  });

  it('composes_models_without_loading_unimplemented_routes', async () => {
    let opened = 0;
    const clients = {administrationClient: {}, open: async () => { ++opened; }};
    const model = new HttpReportModel(clients as Nexus.ServiceClients);
    assert.throws(() => model.generatedReportsModel, /not loaded/);
    await model.load();
    assert.equal(opened, 1);
    assert.ok(model.createReportModel instanceof HttpCreateReportModel);
    assert.ok(model.editScheduledReportModel instanceof
      HttpEditScheduledReportModel);
    assert.ok(model.reportDetailModel instanceof HttpReportDetailModel);
    assert.equal(Request.calls.length, 0);
  });
});
