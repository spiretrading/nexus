import * as assert from 'node:assert/strict';
import { afterEach, beforeEach, describe, it } from 'node:test';
import * as Beam from 'beam';
import { SortableTableHeaderCell } from '../../source/components';
import { ActivityTable, ReportActivityController, ReportActivityModel,
  ReportActivityPage, ReportActivityStatusTag } from
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

class Model extends ReportActivityModel {
  public calls = 0;
  public response: () => Promise<ReportActivityModel.Response> =
    async () => ready(['job']);
  public cancellation: () => Promise<void> = async () => {};

  public async load(): Promise<void> {}

  public async query(): Promise<ReportActivityModel.Response> {
    ++this.calls;
    return this.response();
  }

  public async cancel(): Promise<void> { await this.cancellation(); }

  public async retry(): Promise<void> {}
}

function ready(ids: readonly string[]): ReportActivityModel.Response {
  return {status: ReportActivityModel.ResponseStatus.READY,
    isEmpty: ids.length === 0, totalCount: ids.length,
    activities: ids.map(id => ({id, type: 'Example', parameters: [] as string[],
      status: ReportActivityStatusTag.Status.GENERATING,
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

describe('Report activity refresh', () => {
  const original = globalThis.window;
  let clock: Clock;
  let controller: ReportActivityController;
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
    controller = new ReportActivityController({model});
    synchronousState(controller);
    controller.componentDidMount();
    await flush();
    assert.equal(model.calls, 1);
    let finish: (value: ReportActivityModel.Response) => void;
    model.response = () => new Promise(resolve => { finish = resolve; });
    clock.fire();
    await flush();
    assert.equal(model.calls, 2);
    assert.equal(clock.tasks.size, 0);
    assert.equal(controller.state.response.status,
      ReportActivityModel.ResponseStatus.READY);
    finish(ready([]));
    await flush();
    assert.equal(controller.state.response.activities.length, 0);
    assert.equal(clock.tasks.size, 1);
    controller.componentWillUnmount();
    assert.equal(clock.tasks.size, 0);
  });

  it('continues_after_network_and_response_errors', async () => {
    const errors: unknown[] = [];
    const model = new Model();
    controller = new ReportActivityController({model,
      onActionError: error => errors.push(error)});
    synchronousState(controller);
    controller.componentDidMount();
    await flush();
    model.response = async () => { throw new Error('Offline'); };
    clock.fire();
    await flush();
    assert.equal(errors.length, 1);
    assert.equal(clock.tasks.size, 1);
    assert.equal(controller.state.response.activities.length, 1);
    model.response = async () => ({...ready([]),
      status: ReportActivityModel.ResponseStatus.ERROR});
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
    controller = new ReportActivityController({model});
    synchronousState(controller);
    controller.componentDidMount();
    await flush();
    let finish: (value: ReportActivityModel.Response) => void;
    model.response = () => new Promise(resolve => { finish = resolve; });
    clock.fire();
    await flush();
    let cancelled: () => void;
    model.cancellation = () => new Promise(resolve => { cancelled = resolve; });
    (controller as any).onCancel(['job']);
    await flush();
    assert.equal(clock.tasks.size, 0);
    assert.equal(controller.state.response.activities.length, 0);
    model.response = async () => ready([]);
    cancelled();
    await flush();
    finish(ready(['job']));
    await flush();
    assert.equal(controller.state.response.activities.length, 0);
    assert.equal(clock.tasks.size, 1);
  });

  it('ignores_replaced_models_and_unmounted_responses', async () => {
    const old = new Model();
    let finish: (value: ReportActivityModel.Response) => void;
    old.response = () => new Promise(resolve => { finish = resolve; });
    controller = new ReportActivityController({model: old});
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
    assert.equal(controller.state.response.activities[0].id, 'new');
    model.response = () => new Promise(resolve => { finish = resolve; });
    clock.fire();
    await flush();
    controller.componentWillUnmount();
    finish(ready(['late']));
    await flush();
    assert.equal(clock.tasks.size, 0);
    assert.equal(controller.state.response.activities[0].id, 'new');
  });

  it('drops_selected_jobs_that_disappear', () => {
    const props = {response: ready(['done', 'failed']), pageIndex: 0,
      sort: {column: ActivityTable.Column.DATE_MODIFIED,
        order: SortableTableHeaderCell.SortOrder.NONE}};
    const page = new ReportActivityPage(props);
    synchronousState(page);
    (page as any).onSelectionChange(new Set(['done', 'failed']));
    const response = ready(['failed']);
    (page as any).props = {...props, response: {...response,
      activities: response.activities.map(entry => ({...entry,
        status: ReportActivityStatusTag.Status.FAILED}))}};
    (page as any).updateResponse(false);
    assert.deepEqual([...page.state.selected], ['failed']);
    assert.equal((page as any).isRetryEnabled(), true);
    page.componentWillUnmount();
  });
});
