import * as assert from 'node:assert/strict';
import { afterEach, beforeEach, describe, it } from 'node:test';
import { ReportDetailController, ReportDetailModel, ReportDetailPage } from
  '../../source/pages/report_page/report_detail_page';

class Clock {
  public tasks = new Map<number, () => void>();
  public next = 0;

  public setTimeout = (callback: () => void, delay: number): number => {
    assert.equal(delay, 500);
    const id = ++this.next;
    this.tasks.set(id, callback);
    return id;
  };

  public clearTimeout = (id: number): void => {
    this.tasks.delete(id);
  };

  public fire(): void {
    assert.equal(this.tasks.size, 1);
    const [id, callback] = this.tasks.entries().next().value;
    this.tasks.delete(id);
    callback();
  }
}

class Model extends ReportDetailModel {
  public requests: {id: string;
    resolve: (report: ReportDetailModel.Report) => void;
    reject: (error: Error) => void}[] = [];
  public loading = Promise.resolve();

  public async load(): Promise<void> {
    await this.loading;
  }

  public loadReport(id: string): Promise<ReportDetailModel.Report> {
    return new Promise((resolve, reject) => {
      this.requests.push({id, resolve, reject});
    });
  }
}

function makeReport(id: string): ReportDetailModel.Report {
  return {id, title: `Report ${id}`, parameters: [], filePath: `/report/${id}`};
}

function synchronousState(component: any): void {
  component.setState = (update: any, callback?: () => void) => {
    if(typeof update === 'function') {
      update = update(component.state, component.props);
    }
    component.state = {...component.state, ...update};
    callback?.();
  };
}

async function flush(): Promise<void> {
  await new Promise<void>(resolve => setImmediate(resolve));
}

describe('ReportDetailController', () => {
  const originalWindow = globalThis.window;
  let clock: Clock;
  let model: Model;
  let controller: ReportDetailController;
  let errors: unknown[];
  beforeEach(() => {
    clock = new Clock();
    globalThis.window = clock as unknown as Window & typeof globalThis;
    model = new Model();
    errors = [];
    controller = new ReportDetailController({model, id: '1',
      onError: error => errors.push(error)});
    synchronousState(controller);
  });
  afterEach(() => {
    controller.componentWillUnmount();
    globalThis.window = originalWindow;
  });

  it('fast_response_cancels_placeholder', async () => {
    controller.componentDidMount();
    await flush();
    assert.equal(controller.state.status, null);
    assert.equal(clock.tasks.size, 1);
    model.requests[0].resolve(makeReport('1'));
    await flush();
    assert.equal(controller.render().props.status,
      ReportDetailPage.Status.READY);
    assert.equal(controller.render().props.filePath, '/report/1');
    assert.equal(clock.tasks.size, 0);
  });

  it('slow_failure_and_retry', async () => {
    controller.componentDidMount();
    await flush();
    clock.fire();
    assert.equal(controller.render().props.status,
      ReportDetailPage.Status.IN_PROGRESS);
    assert.equal(controller.render().props.filePath, undefined);
    const error = new Error('Offline');
    model.requests[0].reject(error);
    await flush();
    assert.equal(controller.render().props.status,
      ReportDetailPage.Status.ERROR);
    assert.deepEqual(errors, [error]);
    controller.render().props.onRetry();
    controller.render().props.onRetry();
    await flush();
    assert.equal(model.requests.length, 2);
    assert.equal(controller.state.status, ReportDetailPage.Status.ERROR);
    clock.fire();
    assert.equal(controller.state.status, ReportDetailPage.Status.IN_PROGRESS);
    model.requests[1].resolve(makeReport('1'));
    await flush();
    assert.equal(controller.state.status, ReportDetailPage.Status.READY);
    assert.equal(clock.tasks.size, 0);
  });

  it('ignores_previous_report_and_timer', async () => {
    controller.componentDidMount();
    await flush();
    const oldTimer = clock.tasks.values().next().value;
    const previous = controller.props;
    (controller as any).props = {...previous, id: '2'};
    controller.componentDidUpdate(previous);
    await flush();
    assert.deepEqual(model.requests.map(request => request.id), ['1', '2']);
    model.requests[1].resolve(makeReport('2'));
    await flush();
    model.requests[0].reject(new Error('Old request'));
    oldTimer();
    await flush();
    assert.equal(controller.state.status, ReportDetailPage.Status.READY);
    assert.equal(controller.state.report.id, '2');
    assert.equal(clock.tasks.size, 0);
    assert.deepEqual(errors, []);
  });

  it('unmount_cancels_placeholder_and_ignores_response', async () => {
    controller.componentDidMount();
    await flush();
    controller.componentWillUnmount();
    assert.equal(clock.tasks.size, 0);
    model.requests[0].reject(new Error('Unmounted'));
    await flush();
    assert.equal(controller.state.status, null);
    assert.deepEqual(errors, []);
  });

  it('model_load_failure_can_retry', async () => {
    model.loading = Promise.reject(new Error('Model unavailable'));
    controller.componentDidMount();
    await flush();
    assert.equal(controller.state.status, ReportDetailPage.Status.ERROR);
    assert.equal(model.requests.length, 0);
    assert.equal(clock.tasks.size, 0);
    model.loading = Promise.resolve();
    controller.render().props.onRetry();
    await flush();
    model.requests[0].resolve(makeReport('1'));
    await flush();
    assert.equal(controller.state.status, ReportDetailPage.Status.READY);
  });
});
