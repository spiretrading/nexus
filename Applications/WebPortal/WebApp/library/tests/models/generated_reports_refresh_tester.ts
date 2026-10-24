import * as assert from 'node:assert/strict';
import { afterEach, beforeEach, describe, it } from 'node:test';
import * as Beam from 'beam';
import { LocalAccountGroupQueryModel, SortableTableHeaderCell } from
  '../../source/components';
import { DateRange } from '../../source/models/date_range';
import { GeneratedReportsController, GeneratedReportsModel, ReportTable,
  GeneratedReportsPage, LocalGeneratedReportsModel, ReportStatusTag } from
  '../../source/pages/report_page';

class Clock {
  public tasks = new Map<number, () => void>();
  public next = 0;

  public setTimeout = (callback: () => void, delay: number): number => {
    assert.equal(delay, 5000);
    const id = ++this.next;
    this.tasks.set(id, callback);
    return id;
  };

  public clearTimeout = (id: number): void => { this.tasks.delete(id); };

  public fire(): void {
    assert.equal(this.tasks.size, 1);
    const [id, callback] = this.tasks.entries().next().value;
    this.tasks.delete(id);
    callback();
  }
}

class Model extends LocalGeneratedReportsModel {
  public calls = 0;
  public response: () => Promise<GeneratedReportsModel.Response> =
    async () => ready(['job']);
  public cancellation: () => Promise<void> = async () => {};
  public retrial: () => Promise<void> = async () => {};

  constructor() {
    super([], new LocalAccountGroupQueryModel([]));
  }

  public async query(): Promise<GeneratedReportsModel.Response> {
    ++this.calls;
    return this.response();
  }

  public async cancel(): Promise<void> { await this.cancellation(); }

  public async retry(): Promise<void> { await this.retrial(); }
}

function ready(ids: readonly string[]): GeneratedReportsModel.Response {
  return {status: GeneratedReportsModel.ResponseStatus.READY,
    isEmpty: ids.length === 0, filteredCount: ids.length,
    reports: ids.map(id => ({id, type: 'Example', url: `/reports/${id}`,
      parameters: [] as string[],
      status: ReportStatusTag.Status.GENERATING,
      dateModified: new Beam.DateTime(new Beam.Date(2026, 10, 5))}))};
}

function synchronousState(component: any): void {
  component.setState = (update: any, callback?: () => void) => {
    if(typeof update === 'function') {
      update = update(component.state, component.props);
    }
    if(update) {
      component.state = {...component.state, ...update};
    }
    callback?.();
  };
}

async function flush(): Promise<void> {
  await new Promise<void>(resolve => setImmediate(resolve));
}

describe('Generated reports refresh', () => {
  const original = globalThis.window;
  let clock: Clock;
  let controller: GeneratedReportsController;
  beforeEach(() => {
    clock = new Clock();
    globalThis.window = clock as unknown as Window & typeof globalThis;
  });
  afterEach(() => {
    controller?.componentWillUnmount();
    controller = null;
    globalThis.window = original;
  });

  it('refreshes_without_loading_flicker_or_overlapping_requests', async () => {
    const model = new Model();
    controller = new GeneratedReportsController({model});
    synchronousState(controller);
    controller.componentDidMount();
    await flush();
    assert.equal(model.calls, 1);
    let finish: (value: GeneratedReportsModel.Response) => void;
    model.response = () => new Promise(resolve => { finish = resolve; });
    clock.fire();
    await flush();
    assert.equal(model.calls, 2);
    assert.equal(clock.tasks.size, 0);
    assert.equal(controller.state.response.status,
      GeneratedReportsModel.ResponseStatus.READY);
    finish(ready([]));
    await flush();
    assert.equal(controller.state.response.reports.length, 0);
    assert.equal(clock.tasks.size, 1);
    controller.componentWillUnmount();
    assert.equal(clock.tasks.size, 0);
  });

  it('continues_after_network_and_response_errors', async () => {
    const errors: unknown[] = [];
    const model = new Model();
    controller = new GeneratedReportsController({model,
      onActionError: error => errors.push(error)});
    synchronousState(controller);
    controller.componentDidMount();
    await flush();
    model.response = async () => { throw new Error('Offline'); };
    clock.fire();
    await flush();
    assert.equal(errors.length, 1);
    assert.equal(clock.tasks.size, 1);
    assert.equal(controller.state.response.reports.length, 1);
    model.response = async () => ({...ready([]),
      status: GeneratedReportsModel.ResponseStatus.ERROR});
    clock.fire();
    await flush();
    assert.equal(errors.length, 2);
    assert.equal(clock.tasks.size, 1);
    model.response = async () => ready([]);
    clock.fire();
    await flush();
    assert.equal(controller.state.response.isEmpty, true);
  });

  it('ignores_stale_refresh_during_cancellation', async () => {
    const model = new Model();
    controller = new GeneratedReportsController({model});
    synchronousState(controller);
    controller.componentDidMount();
    await flush();
    let finish: (value: GeneratedReportsModel.Response) => void;
    model.response = () => new Promise(resolve => { finish = resolve; });
    clock.fire();
    await flush();
    let cancelled: () => void;
    model.cancellation = () => new Promise(resolve => { cancelled = resolve; });
    (controller as any).onCancel(['job']);
    await flush();
    assert.equal(clock.tasks.size, 0);
    assert.equal(controller.state.response.reports.length, 0);
    model.response = async () => ready([]);
    cancelled();
    await flush();
    finish(ready(['job']));
    await flush();
    assert.equal(controller.state.response.reports.length, 0);
    assert.equal(clock.tasks.size, 1);
  });

  it('restores_failed_cancellations_before_refresh', async () => {
    const errors: unknown[] = [];
    const model = new Model();
    model.response = async () => ready(['first', 'second', 'third']);
    controller = new GeneratedReportsController({model,
      onActionError: error => errors.push(error)});
    synchronousState(controller);
    controller.componentDidMount();
    await flush();
    let reject: (error: Error) => void;
    model.cancellation = () => new Promise((_, fail) => { reject = fail; });
    model.response = () => new Promise(() => {});
    (controller as any).onCancel(['first', 'third']);
    await flush();
    assert.deepEqual(controller.state.response.reports.map(report =>
      report.id), ['second']);
    assert.equal(controller.state.response.filteredCount, 1);
    reject(new Error('Cancelled unsuccessfully'));
    await flush();
    assert.deepEqual(controller.state.response.reports.map(report =>
      report.id), ['first', 'second', 'third']);
    assert.equal(controller.state.response.filteredCount, 3);
    assert.equal(errors.length, 1);
  });

  it('ignores_stale_refresh_during_retry', async () => {
    const model = new Model();
    controller = new GeneratedReportsController({model});
    synchronousState(controller);
    controller.componentDidMount();
    await flush();
    let finish: (value: GeneratedReportsModel.Response) => void;
    model.response = () => new Promise(resolve => { finish = resolve; });
    clock.fire();
    await flush();
    let retried: () => void;
    model.retrial = () => new Promise(resolve => { retried = resolve; });
    (controller as any).onRetry(['job']);
    await flush();
    assert.equal(clock.tasks.size, 0);
    assert.equal(controller.state.response.reports.length, 1);
    model.response = async () => ready(['retried']);
    retried();
    await flush();
    finish(ready(['stale']));
    await flush();
    assert.equal(controller.state.response.reports[0].id, 'retried');
    assert.equal(clock.tasks.size, 1);
  });

  it('submits_only_eligible_selected_jobs', () => {
    const requests: [string, readonly string[]][] = [];
    const response = ready(['ready', 'failed', 'generating']);
    response.reports = response.reports.map((report, index) => ({...report,
      status: [ReportStatusTag.Status.READY, ReportStatusTag.Status.FAILED,
        ReportStatusTag.Status.GENERATING][index]}));
    const page = new GeneratedReportsPage({response, pageIndex: 0,
      filters: {query: '', dateRange: new DateRange(null, null)},
      sort: {column: ReportTable.Column.DATE_MODIFIED,
        order: SortableTableHeaderCell.SortOrder.NONE},
      recipientModel: new LocalAccountGroupQueryModel([]),
      onDelete: ids => requests.push(['delete', ids]),
      onCancel: ids => requests.push(['cancel', ids]),
      onRetry: ids => requests.push(['retry', ids]),
      onDownload: ids => requests.push(['download', ids]),
      onShare: ids => requests.push(['share', ids])});
    synchronousState(page);
    for(const action of ['onDelete', 'onCancel', 'onRetry', 'onDownload',
        'onOpenShare']) {
      (page as any).onSelectionChange(new Set(
        ['ready', 'failed', 'generating']));
      (page as any)[action]();
    }
    (page as any).onShare([]);
    assert.deepEqual(requests, [['delete', ['ready']],
      ['cancel', ['failed', 'generating']], ['retry', ['failed']],
      ['download', ['ready']], ['share', ['ready']]]);
    page.componentWillUnmount();
  });

  it('ignores_replaced_models_and_unmounted_responses', async () => {
    const old = new Model();
    let finish: (value: GeneratedReportsModel.Response) => void;
    old.response = () => new Promise(resolve => { finish = resolve; });
    controller = new GeneratedReportsController({model: old});
    synchronousState(controller);
    controller.componentDidMount();
    await flush();
    const model = new Model();
    model.response = async () => ready(['new']);
    const previous = controller.props;
    (controller as any).props = {model};
    controller.componentDidUpdate(previous);
    await flush();
    finish(ready(['old']));
    await flush();
    assert.equal(controller.state.response.reports[0].id, 'new');
    model.response = () => new Promise(resolve => { finish = resolve; });
    clock.fire();
    await flush();
    controller.componentWillUnmount();
    finish(ready(['late']));
    await flush();
    assert.equal(clock.tasks.size, 0);
    assert.equal(controller.state.response.reports[0].id, 'new');
  });

  it('drops_selected_jobs_that_disappear', () => {
    const props = {response: ready(['done', 'failed']), pageIndex: 0,
      filters: {query: '', dateRange: new DateRange(null, null)},
      recipientModel: new LocalAccountGroupQueryModel([]),
      sort: {column: ReportTable.Column.DATE_MODIFIED,
        order: SortableTableHeaderCell.SortOrder.NONE}};
    const page = new GeneratedReportsPage(props);
    synchronousState(page);
    (page as any).onSelectionChange(new Set(['done', 'failed']));
    const response = ready(['failed']);
    (page as any).props = {...props, response: {...response,
      reports: response.reports.map(entry => ({...entry,
        status: ReportStatusTag.Status.FAILED}))}};
    (page as any).updateResponse(false);
    assert.deepEqual([...page.state.selected], ['failed']);
    assert.deepEqual((page as any).getSelected(
      [ReportStatusTag.Status.FAILED]), ['failed']);
    (page as any).props = {...props, response: ready([])};
    (page as any).updateResponse(false);
    assert.equal(page.state.selected.size, 0);
    page.componentWillUnmount();
  });
});
