import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import { LocalAccountGroupQueryModel } from '../../source/components';
import { LocalGeneratedReportsModel } from
  '../../source/pages/report_page/generated_reports_page';
import { LocalReportActivityModel } from
  '../../source/pages/report_page/report_activity_page';
import { CompositeReportModel } from
  '../../source/pages/report_page/composite_report_model';
import { LocalScheduledReportsModel, ScheduledReportsModel } from
  '../../source/pages/report_page/scheduled_reports_page';

function makeSchedule(id: string): ScheduledReportsModel.Schedule {
  return {
    id, type: 'Profit and Loss', repeats: true,
    parameters: [
      {label: 'Account / Group', value: 'Alpha'},
      {label: 'Scope', value: 'Canada'}
    ],
    runDate: {value: '2026-10-17', label: 'Oct 17, 2026'}
  };
}

describe('LocalScheduledReportsModel', () => {
  it('requires_load', async () => {
    const schedule = makeSchedule('1');
    const model = new LocalScheduledReportsModel([schedule]);
    const report = new CompositeReportModel(model,
      new LocalGeneratedReportsModel([], new LocalAccountGroupQueryModel([])),
      new LocalReportActivityModel([]));
    assert.equal(model.isLoaded, false);
    assert.equal(report.isLoaded, false);
    assert.throws(() => report.scheduledReportsModel, /Model not loaded/);
    assert.throws(() => model.schedules, /Model not loaded/);
    assert.throws(() => model.runs, /Model not loaded/);
    await assert.rejects(model.loadSchedules({
      filters: {query: ''}, pageIndex: 0
    }), /Model not loaded/);
    await assert.rejects(model.run('1'), /Model not loaded/);
    await assert.rejects(model.duplicate('1'), /Model not loaded/);
    await assert.rejects(model.delete('1'), /Model not loaded/);
    await report.load();
    assert.equal(report.isLoaded, true);
    assert.equal(report.scheduledReportsModel, model);
    assert.equal(model.isLoaded, false);
    await model.load();
    assert.equal(model.isLoaded, true);
    assert.deepEqual(model.schedules, [schedule]);
    assert.deepEqual(model.runs, []);
    await model.run('1');
    await model.load();
    await report.load();
    assert.deepEqual(model.runs, [schedule]);
    assert.deepEqual(model.schedules, [schedule]);
  });

  it('empty_and_no_matches', async () => {
    const empty = new LocalScheduledReportsModel([]);
    await empty.load();
    assert.deepEqual(await empty.loadSchedules({
      filters: {query: ''}, pageIndex: 0
    }), {
      status: ScheduledReportsModel.ResponseStatus.READY,
      isEmpty: true, filteredCount: 0, schedules: []
    });
    const model = new LocalScheduledReportsModel([makeSchedule('1')]);
    await model.load();
    const response = await model.loadSchedules({
      filters: {query: 'missing'}, pageIndex: 0
    });
    assert.equal(response.isEmpty, false);
    assert.equal(response.filteredCount, 0);
    assert.deepEqual(response.schedules, []);
  });

  it('pages_and_filtered_total', async () => {
    const records = Array.from({length: 105}, (_, i) => makeSchedule(String(i)));
    const model = new LocalScheduledReportsModel(records);
    await model.load();
    for(const [pageIndex, start, count] of [[0, 0, 50], [1, 50, 50],
        [2, 100, 5], [3, 150, 0]]) {
      const response = await model.loadSchedules({
        filters: {query: ''}, pageIndex
      });
      assert.equal(response.filteredCount, 105);
      assert.deepEqual(response.schedules, records.slice(start, start + count));
    }
    records[0].type = 'Other';
    const filtered = await model.loadSchedules({
      filters: {query: 'profit'}, pageIndex: 2
    });
    assert.equal(filtered.filteredCount, 105);
    assert.equal(filtered.schedules.length, 5);
    for(const pageIndex of [-1, 1.5, NaN, Infinity]) {
      await assert.rejects(model.loadSchedules({
        filters: {query: ''}, pageIndex
      }), /Invalid page index/);
    }
  });

  it('literal_case_insensitive_fields', async () => {
    const first = makeSchedule('1');
    first.parameters[0].value = 'Alpha [Desk]';
    const second: ScheduledReportsModel.Schedule = {
      ...makeSchedule('2'), type: 'Trading Volume',
      repeats: false, parameters: []};
    const model = new LocalScheduledReportsModel([first, second]);
    await model.load();
    for(const query of ['pRoFiT', 'recurring', 'cOuNt / gRo', '[DESK]', 'canada']) {
      const response = await model.loadSchedules({
        filters: {query}, pageIndex: 0
      });
      assert.deepEqual(response.schedules.map(schedule => schedule.id), ['1']);
    }
    const response = await model.loadSchedules({
      filters: {query: 'Oct 17'}, pageIndex: 0
    });
    assert.equal(response.filteredCount, 2);
  });

  it('isolates_initial_data_and_returned_data', async () => {
    const original = makeSchedule('1');
    const expected = makeSchedule('1');
    const records = [original];
    const model = new LocalScheduledReportsModel(records);
    await model.load();
    original.parameters[0].value = 'Changed';
    original.runDate.label = 'Changed';
    records.length = 0;
    const response = await model.loadSchedules({
      filters: {query: ''}, pageIndex: 0
    });
    assert.deepEqual(response.schedules, [expected]);
    response.schedules[0].parameters[0].label = 'Changed';
    response.schedules[0].runDate.value = '2027-01-01';
    model.schedules[0].type = 'Changed';
    assert.deepEqual(model.schedules, [expected]);
  });

  it('duplicates_deletes_and_records_runs', async () => {
    const original = makeSchedule('1');
    const model = new LocalScheduledReportsModel([original, makeSchedule('2')]);
    await model.load();
    const copy = await model.duplicate('1');
    assert.deepEqual(copy, {...original, id: '3'});
    assert.deepEqual(
      model.schedules.map(schedule => schedule.id), ['1', '3', '2']);
    copy.parameters[0].value = 'Changed';
    copy.runDate.label = 'Changed';
    assert.deepEqual(model.schedules[1], {...original, id: copy.id});
    await model.run(copy.id);
    await model.delete(copy.id);
    assert.deepEqual(model.runs, [{...original, id: copy.id}]);
    model.runs[0].parameters[0].value = 'Changed';
    assert.deepEqual(model.runs, [{...original, id: copy.id}]);
    assert.equal((await model.duplicate('1')).id, '4');
    assert.equal(model.schedules.length, 3);
    await assert.rejects(model.run('missing'), /not found/);
    await assert.rejects(model.duplicate('missing'), /not found/);
    await assert.rejects(model.delete('missing'), /not found/);
  });

  it('report_model_composition', async () => {
    const schedules = new LocalScheduledReportsModel([makeSchedule('1')]);
    const generated = new LocalGeneratedReportsModel([],
      new LocalAccountGroupQueryModel([]));
    const activity = new LocalReportActivityModel([]);
    const report = new CompositeReportModel(schedules, generated, activity);
    assert.throws(() => report.reportActivityModel, /Model not loaded/);
    await report.load();
    await schedules.load();
    assert.equal(report.scheduledReportsModel, schedules);
    assert.equal(report.generatedReportsModel, generated);
    assert.equal(report.reportActivityModel, activity);
    assert.equal(activity.isLoaded, false);
    await report.scheduledReportsModel.delete('1');
    assert.deepEqual(schedules.schedules, []);
  });
});
