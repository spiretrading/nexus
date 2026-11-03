import * as assert from 'node:assert/strict';
import { afterEach, describe, it } from 'node:test';
import { LocalScheduledReportsModel, ScheduledReportsController,
  ScheduledReportsModel } from
  '../../source/pages/report_page/scheduled_reports_page';

class Model extends LocalScheduledReportsModel {
  public queries = 0;
  public pending: Promise<void>;

  public async query(submission: ScheduledReportsModel.Submission):
      Promise<ScheduledReportsModel.Response> {
    ++this.queries;
    await this.pending;
    return super.query(submission);
  }
}

function makeSchedules(): ScheduledReportsModel.Schedule[] {
  return Array.from({length: 51}, (_, index): ScheduledReportsModel.Schedule =>
    ({id: `schedule-${index}`,
    type: `Report ${index}`, repeats: true, parameters: [],
    runDate: {value: '2099-01-01', label: 'Jan 01, 2099'}}));
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

describe('ScheduledReportsController', () => {
  let controller: ScheduledReportsController;
  afterEach(() => {
    controller?.componentWillUnmount();
    controller = null;
  });

  it('duplicate_appears_first_and_stays_there_after_refresh', async () => {
    const model = new Model(makeSchedules());
    controller = new ScheduledReportsController({model});
    synchronousState(controller);
    controller.componentDidMount();
    await flush();
    await controller.render().props.onDuplicate('schedule-20');
    const response = controller.state.response;
    assert.equal(response.filteredCount, 52);
    assert.equal(response.schedules.length, 50);
    assert.deepEqual(response.schedules[0],
      {...makeSchedules()[20], id: '1'});
    assert.equal(response.schedules[49].id, 'schedule-48');
    assert.equal(model.queries, 1);
    controller.render().props.onSubmit(controller.state.submission);
    await flush();
    assert.deepEqual(controller.state.response, response);
  });

  it('duplicate_on_later_page_refreshes_without_changing_page', async () => {
    const model = new Model(makeSchedules());
    controller = new ScheduledReportsController({model});
    synchronousState(controller);
    controller.componentDidMount();
    await flush();
    controller.render().props.onSubmit({filters: {query: ''}, pageIndex: 1});
    await flush();
    const original = controller.state.response;
    let finish: () => void;
    model.pending = new Promise(resolve => { finish = resolve; });
    const duplication = controller.render().props.onDuplicate('schedule-50');
    await flush();
    assert.equal(controller.state.response, original);
    assert.equal(controller.state.response.status,
      ScheduledReportsModel.ResponseStatus.READY);
    finish();
    await duplication;
    assert.equal(controller.state.submission.pageIndex, 1);
    assert.equal(controller.state.response.filteredCount, 52);
    assert.deepEqual(controller.state.response.schedules.map(entry => entry.id),
      ['schedule-49', 'schedule-50']);
    assert.equal(model.schedules[0].id, '1');
    controller.render().props.onSubmit(controller.state.submission);
    await flush();
    assert.deepEqual(controller.state.response.schedules.map(entry => entry.id),
      ['schedule-49', 'schedule-50']);
  });

  it('duplicate_refreshes_filtered_results_in_display_order', async () => {
    const schedules = makeSchedules();
    schedules[10].type = 'Needle';
    schedules[40].type = 'Needle';
    const model = new Model(schedules);
    controller = new ScheduledReportsController({model});
    synchronousState(controller);
    controller.componentDidMount();
    await flush();
    controller.render().props.onSubmit({
      filters: {query: 'needle'}, pageIndex: 0});
    await flush();
    await controller.render().props.onDuplicate('schedule-40');
    assert.equal(model.queries, 3);
    assert.equal(controller.state.submission.filters.query, 'needle');
    assert.equal(controller.state.response.filteredCount, 3);
    assert.deepEqual(controller.state.response.schedules.map(entry => entry.id),
      ['1', 'schedule-10', 'schedule-40']);
  });
});
