import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import * as Beam from 'beam';
import { SortableTableHeaderCell } from '../../source/components';
import { ActivityTable, LocalReportActivityModel, ReportActivityModel,
  ReportActivityStatusTag } from '../../source/pages/report_page';

function makeActivities(count: number): ActivityTable.Activity[] {
  return Array.from({length: count}, (_, index) => ({id: String(index + 1),
    type: 'Profit and Loss', parameters: [`Group ${index + 1}`],
    status: ReportActivityStatusTag.Status.FAILED,
    dateModified: new Beam.DateTime(new Beam.Date(2024, 2, index % 29 + 1))}));
}

function submission(): ReportActivityModel.Submission {
  return {sort: {column: ActivityTable.Column.DATE_MODIFIED,
    order: SortableTableHeaderCell.SortOrder.NONE}, pageIndex: 0};
}

describe('LocalReportActivityModel', () => {
  it('sorts_timestamps_and_preserves_time_of_day', async () => {
    const activities = makeActivities(2).map((activity, index) => ({...activity,
      dateModified: Beam.DateTime.fromJson([
        '20261005T235959.125', '20261005T000001.125'][index])}));
    const model = new LocalReportActivityModel(activities);
    await model.load();
    const request = submission();
    request.sort.order = SortableTableHeaderCell.SortOrder.ASCENDING;
    let response = await model.query(request);
    assert.deepEqual(response.activities.map(entry => entry.id), ['2', '1']);
    assert.equal(response.activities[1].dateModified.toJson(),
      '20261005T235959.125');
    assert.equal(model.activities[0].dateModified.toJson(),
      '20261005T235959.125');
    request.sort.order = SortableTableHeaderCell.SortOrder.DESCENDING;
    response = await model.query(request);
    assert.deepEqual(response.activities.map(entry => entry.id), ['1', '2']);
  });

  it('retries_with_utc_timestamp', async context => {
    context.mock.timers.enable({apis: ['Date'],
      now: Date.UTC(2026, 9, 7, 1, 2, 3, 125)});
    const model = new LocalReportActivityModel(makeActivities(1));
    await model.load();
    await model.retry(['1']);
    assert.equal(model.activities[0].dateModified.toJson(),
      '20261007T010203.125');
  });

  it('requires_load', async () => {
    const model = new LocalReportActivityModel(makeActivities(1));
    assert.throws(() => model.activities);
    await assert.rejects(model.query(submission()));
    await assert.rejects(model.cancel(['1']));
    await assert.rejects(model.retry(['1']));
    await model.load();
    assert.equal(model.activities.length, 1);
  });

  it('paginates_cancels_and_retries', async () => {
    const model = new LocalReportActivityModel(makeActivities(105));
    await model.load();
    const request = submission();
    let response = await model.query(request);
    assert.equal(response.activities.length, 50);
    assert.equal(response.totalCount, 105);
    request.pageIndex = 2;
    response = await model.query(request);
    assert.deepEqual(response.activities.map(entry => entry.id),
      ['101', '102', '103', '104', '105']);
    await model.retry(['101']);
    response = await model.query(request);
    assert.equal(response.activities[0].status,
      ReportActivityStatusTag.Status.GENERATING);
    assert.equal(response.activities[0].dateModified.date.toJson(),
      new Date().toISOString().slice(0, 10).replace(/-/g, ''));
    assert.equal(response.activities[1].status,
      ReportActivityStatusTag.Status.FAILED);
    await model.cancel(['101', '102', '103', '104', '105']);
    assert.equal((await model.query(request)).totalCount, 100);
    assert.equal((await model.query(request)).isEmpty, false);
    await model.cancel(model.activities.map(entry => entry.id));
    response = await model.query(submission());
    assert.equal(response.isEmpty, true);
    assert.equal(response.totalCount, 0);
    assert.deepEqual(response.activities, []);
  });

  it('sorts_and_preserves_snapshots', async () => {
    const activities = [
      {...makeActivities(1)[0], id: '1', type: 'Zulu', parameters: ['A'],
        dateModified: new Beam.DateTime(new Beam.Date(2024, 2, 29))},
      {...makeActivities(1)[0], id: '2', type: 'Alpha', parameters: ['Z'],
        status: ReportActivityStatusTag.Status.GENERATING,
        dateModified: new Beam.DateTime(new Beam.Date(2024, 3, 1))}];
    const model = new LocalReportActivityModel(activities);
    activities[0].parameters[0] = 'changed';
    await model.load();
    const request = submission();
    for(const column of [ActivityTable.Column.TYPE,
        ActivityTable.Column.STATUS]) {
      request.sort = {column,
        order: SortableTableHeaderCell.SortOrder.ASCENDING};
      assert.equal((await model.query(request)).activities[0].id, '2');
      request.sort.order = SortableTableHeaderCell.SortOrder.DESCENDING;
      assert.equal((await model.query(request)).activities[0].id, '1');
    }
    for(const column of [ActivityTable.Column.PARAMETERS,
        ActivityTable.Column.DATE_MODIFIED]) {
      request.sort = {column,
        order: SortableTableHeaderCell.SortOrder.ASCENDING};
      assert.equal((await model.query(request)).activities[0].id, '1');
      request.sort.order = SortableTableHeaderCell.SortOrder.DESCENDING;
      assert.equal((await model.query(request)).activities[0].id, '2');
    }
    const snapshot = model.activities;
    (snapshot[0].parameters as string[])[0] = 'mutated';
    const response = await model.query(submission());
    (response.activities[0].parameters as string[])[0] = 'mutated';
    assert.equal(model.activities[0].parameters[0], 'A');
    await model.retry(['2']);
    assert.equal(model.activities[1].dateModified.compare(
      new Beam.DateTime(new Beam.Date(2024, 3, 1))), 0);
    assert.deepEqual((await model.query(submission())).activities.
      map(entry => entry.id), ['1', '2']);
  });
});
