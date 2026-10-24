import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import * as Beam from 'beam';
import { LocalAccountGroupQueryModel } from '../../source/components';
import { Interval, LocalTickerQueryModel, ScopeQueryModel } from
  '../../source/models';
import { LocalEditScheduledReportModel, LocalScheduledReportsModel,
  ReportDefinition, ReportFormTemplate, ScheduledReportsModel } from
  '../../source/pages/report_page';

function definition(): ReportDefinition {
  return ReportDefinition.fromJson({id: 'test', name: 'Test', parameters: [
    {name: 'accounts', label: 'Accounts', type: 'DirectoryEntryList',
      default: [Beam.DirectoryEntry.STAR_DIRECTORY.toJson()]},
    {name: 'count', label: 'Count', type: 'Integer', default: 0}],
    output: {media_type: 'text/csv', extension: 'csv'}});
}

function value(): ReportFormTemplate.Value {
  return {...ReportFormTemplate.makeValue(definition()), scheduled: true,
    scheduleDateTime: new Beam.DateTime(new Beam.Date(2026, 1, 2),
      Beam.Duration.HOUR.multiply(9)), repeats: true,
    repeatInterval: new Interval(2, Interval.Unit.MONTH)};
}

function model(initial: ReportFormTemplate.Value):
    LocalEditScheduledReportModel {
  return new LocalEditScheduledReportModel([definition()],
    new LocalAccountGroupQueryModel([]),
    new ScopeQueryModel(new LocalTickerQueryModel([])),
    new Map([['first', initial], ['second', initial]]));
}

describe('LocalEditScheduledReportModel', () => {
  it('requires_load_and_existing_schedule', async () => {
    const source = model(value());
    assert.throws(() => source.reports);
    assert.throws(() => source.accountModel);
    assert.throws(() => source.scopeModel);
    assert.throws(() => source.set('new', value()));
    assert.throws(() => source.delete('first'));
    await assert.rejects(source.loadReport('first'));
    await assert.rejects(source.submit('first', value()));
    await source.load();
    assert.equal(source.reports[0].id, 'test');
    assert.ok(source.accountModel);
    assert.ok(source.scopeModel);
    await assert.rejects(source.loadReport('missing'));
    await assert.rejects(source.submit('missing', value()));
  });

  it('saves_independent_configurations', async () => {
    const initial = value();
    const source = model(initial);
    await source.load();
    (initial.parameters.accounts as Beam.DirectoryEntry[]).splice(0);
    const draft = await source.loadReport('first');
    assert.equal((draft.parameters.accounts as Beam.DirectoryEntry[]).
      length, 1);
    draft.parameters = {...draft.parameters, count: 9};
    draft.recipients = [Beam.DirectoryEntry.makeAccount(1, 'Alice')];
    await source.submit('first', draft);
    (draft.parameters.accounts as Beam.DirectoryEntry[]).splice(0);
    (draft.recipients as Beam.DirectoryEntry[]).splice(0);
    const saved = await source.loadReport('first');
    assert.equal(saved.parameters.count, 9);
    assert.equal((saved.parameters.accounts as Beam.DirectoryEntry[]).
      length, 1);
    assert.equal(saved.recipients[0].name, 'Alice');
    assert.equal(saved.scheduleDateTime.date.year, 2026);
    assert.equal(saved.repeatInterval.count, 2);
    assert.equal(saved.repeatInterval.unit, Interval.Unit.MONTH);
    assert.equal((await source.loadReport('second')).parameters.count, 0);
    source.set('copy', saved);
    source.delete('first');
    await assert.rejects(source.loadReport('first'));
    assert.equal((await source.loadReport('copy')).parameters.count, 9);
  });

  it('updates_list_entry_without_reordering', async () => {
    const entry: ScheduledReportsModel.Schedule = {id: 'first', type: 'Test',
      parameters: [], repeats: false,
      runDate: {value: '2026-01-02', label: 'Jan 02, 2026'}};
    const source = new LocalScheduledReportsModel([
      entry, {...entry, id: 'second'}]);
    assert.throws(() => source.update(entry));
    await source.load();
    const changed = {...entry, repeats: true,
      parameters: [{label: 'Count', value: '9'}]};
    source.update(changed);
    changed.parameters[0].value = 'mutated';
    assert.deepEqual(source.schedules.map(item => item.id),
      ['first', 'second']);
    assert.equal(source.schedules[0].parameters[0].value, '9');
    assert.equal(source.schedules[0].repeats, true);
    assert.equal(source.schedules[1].repeats, false);
    assert.throws(() => source.update({...entry, id: 'missing'}));
  });
});
