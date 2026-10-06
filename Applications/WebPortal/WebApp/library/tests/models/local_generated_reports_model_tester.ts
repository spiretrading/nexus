import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import * as Beam from 'beam';
import { LocalAccountGroupQueryModel, SortableTableHeaderCell } from
  '../../source/components';
import { DateRange } from '../../source/models/date_range';
import { GeneratedReportsModel, LocalGeneratedReportsModel, ReportTable } from
  '../../source/pages/report_page/generated_reports_page';

function makeReports(count: number): ReportTable.Report[] {
  return Array.from({length: count}, (_, index) => ({id: String(index + 1),
    type: 'Profit and Loss', parameters: [`Group ${index + 1}`, 'Canada'],
    url: `/reports/${index + 1}`, dateCreated: new Beam.Date(2024, 2,
      index % 29 + 1)}));
}

function submission(): GeneratedReportsModel.Submission {
  return {filters: {query: '', dateRange: new DateRange(null, null)},
    sort: {column: ReportTable.Column.DATE_CREATED,
      order: SortableTableHeaderCell.SortOrder.NONE}, pageIndex: 0};
}

describe('LocalGeneratedReportsModel', () => {
  it('requires_load', async () => {
    const model = new LocalGeneratedReportsModel(makeReports(1),
      new LocalAccountGroupQueryModel([]));
    assert.throws(() => model.reports);
    assert.throws(() => model.recipientModel);
    assert.throws(() => model.shares);
    assert.throws(() => model.downloads);
    await assert.rejects(model.query(submission()));
    await assert.rejects(model.delete(['1']));
    await assert.rejects(model.share(['1'], []));
    await assert.rejects(model.download(['1']));
    await model.load();
    assert.equal(model.reports.length, 1);
  });

  it('filters_dates_queries_and_pages', async () => {
    const model = new LocalGeneratedReportsModel(makeReports(105),
      new LocalAccountGroupQueryModel([]));
    await model.load();
    const request = submission();
    let response = await model.query(request);
    assert.equal(response.filteredCount, 105);
    assert.equal(response.reports.length, 50);
    request.pageIndex = 2;
    response = await model.query(request);
    assert.deepEqual(response.reports.map(report => report.id),
      ['101', '102', '103', '104', '105']);
    request.pageIndex = 0;
    request.filters.query = 'GROUP 105';
    response = await model.query(request);
    assert.equal(response.filteredCount, 1);
    assert.equal(response.reports[0].id, '105');
    request.filters.query = '';
    request.filters.dateRange = new DateRange(
      new Beam.Date(2024, 2, 29), new Beam.Date(2024, 2, 29));
    response = await model.query(request);
    assert.deepEqual(response.reports.map(report => report.id),
      ['29', '58', '87']);
    request.filters.dateRange = new DateRange(null, new Beam.Date(2024, 2, 1));
    assert.equal((await model.query(request)).filteredCount, 4);
    request.filters.dateRange = new DateRange(new Beam.Date(2024, 2, 29), null);
    assert.equal((await model.query(request)).filteredCount, 3);
    request.filters.query = 'no match';
    response = await model.query(request);
    assert.equal(response.filteredCount, 0);
    assert.equal(response.isEmpty, false);
    await model.delete(makeReports(105).map(report => report.id));
    assert.equal((await model.query(request)).isEmpty, true);
  });

  it('sorts_and_preserves_snapshots', async () => {
    const reports = [
      {...makeReports(1)[0], id: '1', type: 'Zulu', parameters: ['A'],
        dateCreated: new Beam.Date(2024, 2, 29)},
      {...makeReports(1)[0], id: '2', type: 'Alpha', parameters: ['Z'],
        dateCreated: new Beam.Date(2024, 3, 1)}];
    const model = new LocalGeneratedReportsModel(reports,
      new LocalAccountGroupQueryModel([]));
    reports[0].parameters[0] = 'mutated';
    await model.load();
    const request = submission();
    request.sort = {column: ReportTable.Column.TYPE,
      order: SortableTableHeaderCell.SortOrder.ASCENDING};
    assert.equal((await model.query(request)).reports[0].id, '2');
    request.sort.order = SortableTableHeaderCell.SortOrder.DESCENDING;
    assert.equal((await model.query(request)).reports[0].id, '1');
    request.sort.column = ReportTable.Column.PARAMETERS;
    assert.equal((await model.query(request)).reports[0].id, '2');
    request.sort.column = ReportTable.Column.DATE_CREATED;
    assert.equal((await model.query(request)).reports[0].id, '2');
    request.sort.order = SortableTableHeaderCell.SortOrder.NONE;
    assert.equal((await model.query(request)).reports[0].id, '1');
    const snapshot = model.reports;
    (snapshot[0].parameters as string[])[0] = 'changed';
    assert.equal(model.reports[0].parameters[0], 'A');
    const ids = ['1'];
    const recipients = [Beam.DirectoryEntry.makeAccount(1, 'Alice')];
    await model.share(ids, recipients);
    await model.download(ids);
    ids.push('2');
    recipients.push(Beam.DirectoryEntry.makeAccount(2, 'Bob'));
    assert.deepEqual(model.shares[0].ids, ['1']);
    assert.equal(model.shares[0].recipients.length, 1);
    assert.deepEqual(model.downloads, [['1']]);
    await model.delete(['1']);
    assert.deepEqual(model.reports.map(report => report.id), ['2']);
  });
});
