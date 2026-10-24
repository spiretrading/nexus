import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import { LocalReportDetailModel, ReportDetailModel } from
  '../../source/pages/report_page/report_detail_page';

function report(): ReportDetailModel.Report {
  return {id: '1', title: 'Profit and Loss',
    parameters: [{label: 'Account / Group', value: 'Alpha Group'}],
    status: ReportDetailModel.Status.READY, filePath: '/reports/1.csv',
    content: 'Ticker,Profit\nABX.TSX,100\n'};
}

describe('LocalReportDetailModel', () => {
  it('requires_load_and_existing_report', async () => {
    const model = new LocalReportDetailModel([report()]);
    await assert.rejects(model.loadReport('1'));
    assert.throws(() => model.delete('1'));
    assert.throws(() => model.store(report()));
    await model.load();
    assert.deepEqual(await model.loadReport('1'), report());
    await assert.rejects(model.loadReport('missing'));
    model.delete('1');
    await assert.rejects(model.loadReport('1'));
  });

  it('stores_job_status_changes_as_snapshots', async () => {
    const model = new LocalReportDetailModel([]);
    await model.load();
    const pending: ReportDetailModel.Report = {...report(),
      status: ReportDetailModel.Status.IN_PROGRESS,
      filePath: '', content: null};
    model.store(pending);
    pending.title = 'Changed';
    assert.equal((await model.loadReport('1')).title, 'Profit and Loss');
    assert.equal((await model.loadReport('1')).status,
      ReportDetailModel.Status.IN_PROGRESS);
    model.store(report());
    assert.deepEqual(await model.loadReport('1'), report());
  });

  it('isolates_metadata_and_preserves_optional_content', async () => {
    const original = report();
    const binary: ReportDetailModel.Report = {...report(), id: '2',
      status: ReportDetailModel.Status.READY, filePath: '/reports/2.zip',
      content: null};
    const model = new LocalReportDetailModel([original, binary]);
    original.parameters[0].value = 'Changed';
    original.title = 'Changed';
    await model.load();
    const loaded = await model.loadReport('1');
    assert.deepEqual(loaded, report());
    loaded.parameters[0].label = 'Changed';
    loaded.filePath = '/changed';
    assert.deepEqual(await model.loadReport('1'), report());
    assert.deepEqual(await model.loadReport('2'), binary);
  });
});
