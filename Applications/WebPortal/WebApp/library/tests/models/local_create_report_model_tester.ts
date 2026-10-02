import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import * as Beam from 'beam';
import { LocalAccountGroupQueryModel } from '../../source/components';
import { Interval, LocalTickerQueryModel, ScopeQueryModel } from
  '../../source/models';
import { LocalCreateReportModel, LocalReportActivityModel,
  LocalScheduledReportsModel, ReportActivityStatusTag, ReportDefinition,
  ReportFormTemplate } from '../../source/pages/report_page';

function definition(): ReportDefinition {
  return ReportDefinition.fromJson({id: 'test', name: 'Test', parameters: [
    {name: 'accounts', label: 'Accounts', type: 'DirectoryEntryList',
      default: [Beam.DirectoryEntry.STAR_DIRECTORY.toJson()]},
    {name: 'count', label: 'Count', type: 'Integer', default: 0}],
    output: {media_type: 'text/csv', extension: 'csv'}});
}

function model(): LocalCreateReportModel {
  return new LocalCreateReportModel([definition()],
    new LocalAccountGroupQueryModel([]),
    new ScopeQueryModel(new LocalTickerQueryModel([])));
}

describe('LocalCreateReportModel', () => {
  it('requires_load', async () => {
    const source = model();
    assert.throws(() => source.reports);
    assert.throws(() => source.accountModel);
    assert.throws(() => source.scopeModel);
    assert.throws(() => source.requests);
    await assert.rejects(source.submit(
      ReportFormTemplate.makeValue(definition())));
    await source.load();
    assert.equal(source.reports[0].id, 'test');
    assert.ok(source.accountModel);
    assert.ok(source.scopeModel);
  });

  it('records_independent_immediate_and_scheduled_requests', async () => {
    const source = model();
    await source.load();
    const value = ReportFormTemplate.makeValue(source.reports[0]);
    assert.equal(await source.submit(value), '1');
    value.scheduled = true;
    value.scheduleDateTime = new Beam.DateTime(new Beam.Date(2099, 1, 2),
      Beam.Duration.HOUR.multiply(9));
    value.repeats = true;
    value.repeatInterval = new Interval(2, Interval.Unit.MONTH);
    assert.equal(await source.submit(value), '2');
    (value.parameters.accounts as Beam.DirectoryEntry[]).push(
      Beam.DirectoryEntry.makeAccount(1, 'Alice'));
    value.recipients = [Beam.DirectoryEntry.makeAccount(2, 'Bob')];
    const requests = source.requests;
    assert.equal(requests[0].scheduled, false);
    assert.equal(requests[1].scheduled, true);
    assert.equal(requests[1].repeatInterval.unit, Interval.Unit.MONTH);
    assert.equal(requests[1].repeatInterval.count, 2);
    assert.equal(requests[1].scheduleDateTime.compare(
      new Beam.DateTime(new Beam.Date(2099, 1, 2),
        Beam.Duration.HOUR.multiply(9))), 0);
    assert.equal((requests[1].parameters.accounts as Beam.DirectoryEntry[]).
      length, 1);
    assert.deepEqual(requests[1].recipients, []);
    (requests[1].parameters.accounts as Beam.DirectoryEntry[]).splice(0);
    const storedAccounts = source.requests[1].parameters.accounts as
      Beam.DirectoryEntry[];
    assert.equal(storedAccounts.length, 1);
    assert.equal(source.requests[1].parameters.count, 0);
    assert.equal(storedAccounts[0].id, Beam.DirectoryEntry.STAR_DIRECTORY.id);
  });

  it('inserts_created_entries', async () => {
    const activity = {type: 'Report', parameters: ['*'],
      status: ReportActivityStatusTag.Status.GENERATING,
      dateModified: new Beam.Date(2026, 10, 2)};
    const activities = new LocalReportActivityModel([{...activity, id: '1'}]);
    assert.throws(() => activities.add(activity));
    await activities.load();
    assert.equal(activities.add(activity), '2');
    activity.parameters.push('mutated');
    assert.deepEqual(activities.activities.map(entry => entry.id), ['2', '1']);
    assert.deepEqual(activities.activities[0].parameters, ['*']);
    const schedule = {type: 'Report',
      parameters: [{label: 'Scope', value: '*'}],
      repeats: true, runDate: {value: '2099-01-02', label: 'Jan 02, 2099'}};
    const schedules = new LocalScheduledReportsModel([{...schedule, id: '1'}]);
    assert.throws(() => schedules.add(schedule));
    await schedules.load();
    assert.equal(schedules.add(schedule), '2');
    schedule.parameters[0].value = 'mutated';
    assert.deepEqual(schedules.schedules.map(entry => entry.id), ['2', '1']);
    assert.equal(schedules.schedules[0].parameters[0].value, '*');
    assert.equal((await schedules.duplicate('1')).id, '3');
  });
});
