import * as React from 'react';
import { ScheduledReportsModel } from './scheduled_reports_model';
import { ScheduledReportsPage } from './scheduled_reports_page';

interface Properties {

  /** The model providing scheduled reports and their actions. */
  model: ScheduledReportsModel;

  /** Called to navigate to the Create Report Page. */
  onNewReport?: () => void;

  /** Called when a report action or its background refresh fails. */
  onActionError?: (error: unknown) => void;
}

interface State {
  submission: ScheduledReportsModel.Submission;
  response: ScheduledReportsModel.Response;
}

/** Loads scheduled reports and applies the page's requested actions. */
export class ScheduledReportsController extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      submission: {filters: {query: ''}, pageIndex: 0},
      response: {
        status: ScheduledReportsModel.ResponseStatus.IN_PROGRESS,
        isEmpty: false, filteredCount: 0, schedules: []
      }
    };
    this.mounted = false;
    this.request = 0;
    this.generation = 0;
    this.loadingModel = null;
    this.deletions = new Map();
    this.deletionOrder = [];
    this.refreshRequired = false;
  }

  public render(): JSX.Element {
    return <ScheduledReportsPage key={this.generation}
      {...this.state.submission} response={this.state.response}
      onSubmit={this.onSubmit} onNewReport={this.props.onNewReport}
      onRun={this.onRun} onDuplicate={this.onDuplicate}
      onDelete={this.onDelete}/>;
  }

  public componentDidMount(): void {
    this.mounted = true;
    this.submit(this.state.submission, false);
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.model !== this.props.model) {
      ++this.generation;
      this.loadingModel = null;
      this.deletions = new Map();
      this.deletionOrder = [];
      this.refreshRequired = false;
      this.submit({filters: {query: ''}, pageIndex: 0}, false);
    }
  }

  public componentWillUnmount(): void {
    this.mounted = false;
    ++this.generation;
    ++this.request;
  }

  private loadModel(): Promise<void> {
    if(!this.loadingModel) {
      const model = this.props.model;
      const loading = Promise.resolve().then(() => model.load());
      this.loadingModel = loading;
      loading.catch(() => {
        if(this.loadingModel === loading) {
          this.loadingModel = null;
        }
      });
    }
    return this.loadingModel;
  }

  private async submit(submission: ScheduledReportsModel.Submission,
      isBackground: boolean): Promise<void> {
    const request = ++this.request;
    const model = this.props.model;
    this.refreshRequired = false;
    if(!isBackground) {
      this.setState({submission, response: {
        ...this.state.response,
        status: ScheduledReportsModel.ResponseStatus.IN_PROGRESS
      }});
    }
    try {
      await this.loadModel();
      await Promise.allSettled(this.deletions.values());
      if(!this.isCurrent(request, model)) {
        return;
      }
      const response = await model.loadSchedules(submission);
      if(!this.isCurrent(request, model)) {
        return;
      }
      if(isBackground &&
          response.status === ScheduledReportsModel.ResponseStatus.ERROR) {
        this.props.onActionError?.(
          new Error('Scheduled reports request failed.'));
        return;
      }
      const lastPage = Math.max(0, Math.ceil(
        response.filteredCount / ScheduledReportsModel.PAGE_SIZE) - 1);
      if(response.status === ScheduledReportsModel.ResponseStatus.READY &&
          submission.pageIndex > lastPage) {
        await this.submit({...submission, pageIndex: lastPage}, false);
        return;
      }
      this.setState({response});
    } catch(error) {
      if(this.isCurrent(request, model)) {
        if(isBackground) {
          this.props.onActionError?.(error);
        } else {
          this.setState({response: {
            status: ScheduledReportsModel.ResponseStatus.ERROR,
            isEmpty: false, filteredCount: 0, schedules: []
          }});
        }
      }
    }
  }

  private isCurrent(request: number, model: ScheduledReportsModel): boolean {
    return this.mounted && request === this.request &&
      model === this.props.model;
  }

  private async execute<T>(action: (model: ScheduledReportsModel) =>
      Promise<T>): Promise<T> {
    const model = this.props.model;
    const generation = this.generation;
    try {
      await this.loadModel();
      if(!this.mounted || generation !== this.generation) {
        return;
      }
      const result = await action(model);
      if(this.mounted && generation === this.generation) {
        return result;
      }
    } catch(error) {
      if(this.mounted && generation === this.generation) {
        this.props.onActionError?.(error);
      }
    }
  }

  private async deleteReport(schedule: ScheduledReportsModel.Schedule):
      Promise<void> {
    const model = this.props.model;
    const generation = this.generation;
    try {
      await this.loadModel();
      if(!this.mounted || generation !== this.generation) {
        return;
      }
      await model.delete(schedule.id);
    } catch(error) {
      if(this.mounted && generation === this.generation) {
        this.setState(state => {
          const schedules = state.response.schedules.slice();
          const position = this.deletionOrder.indexOf(schedule.id);
          const index = schedules.findIndex(entry =>
            this.deletionOrder.indexOf(entry.id) > position);
          if(index < 0) {
            schedules.push(schedule);
          } else {
            schedules.splice(index, 0, schedule);
          }
          return {response: {...state.response, schedules, isEmpty: false,
            filteredCount: state.response.filteredCount + 1}};
        });
        this.props.onActionError?.(error);
      }
    } finally {
      if(this.mounted && generation === this.generation) {
        this.deletions.delete(schedule.id);
        if(this.deletions.size === 0) {
          const {response, submission} = this.state;
          if(this.refreshRequired) {
            this.submit(submission, false);
          } else if(response.status ===
              ScheduledReportsModel.ResponseStatus.READY) {
            const lastPage = Math.max(0, Math.ceil(
              response.filteredCount / ScheduledReportsModel.PAGE_SIZE) - 1);
            if(submission.pageIndex > lastPage) {
              this.submit({...submission, pageIndex: lastPage}, false);
            } else if(response.filteredCount === 0 &&
                submission.filters.query !== '') {
              this.submit(submission, true);
            } else if(response.filteredCount >
                submission.pageIndex * ScheduledReportsModel.PAGE_SIZE +
                  response.schedules.length &&
                response.schedules.length < ScheduledReportsModel.PAGE_SIZE) {
              this.submit(submission, true);
            }
          }
        }
      }
    }
  }

  private onSubmit = (submission: ScheduledReportsModel.Submission) => {
    this.submit(submission, false);
  };

  private onRun = (id: string) => {
    this.execute(model => model.run(id));
  };

  private onDuplicate = async (id: string) => {
    const generation = this.generation;
    const request = this.request;
    const originalResponse = this.state.response;
    const schedule = await this.execute(model => model.duplicate(id));
    if(!schedule) {
      return;
    }
    await Promise.allSettled(this.deletions.values());
    if(!this.mounted || generation !== this.generation) {
      return;
    }
    const {response, submission} = this.state;
    const index = response.schedules.findIndex(entry => entry.id === id);
    if(request === this.request && response === originalResponse &&
        submission.filters.query === '' &&
        response.status === ScheduledReportsModel.ResponseStatus.READY &&
        index >= 0) {
      ++this.request;
      const schedules = response.schedules.slice();
      schedules.splice(index + 1, 0, schedule);
      this.setState({response: {...response, isEmpty: false,
        filteredCount: response.filteredCount + 1,
        schedules: schedules.slice(0, ScheduledReportsModel.PAGE_SIZE)}});
      if(schedules.length < ScheduledReportsModel.PAGE_SIZE &&
          response.filteredCount + 1 >
            submission.pageIndex * ScheduledReportsModel.PAGE_SIZE +
              schedules.length) {
        await this.submit(submission, true);
      }
    } else {
      await this.submit(submission,
        response.status === ScheduledReportsModel.ResponseStatus.READY);
    }
  };

  private onDelete = (id: string) => {
    if(this.deletions.has(id)) {
      return;
    }
    const schedule = this.state.response.schedules.
      find(entry => entry.id === id);
    if(!schedule) {
      return;
    }
    if(this.deletions.size === 0) {
      this.deletionOrder = this.state.response.schedules.map(entry => entry.id);
    }
    this.refreshRequired ||= this.state.response.status ===
      ScheduledReportsModel.ResponseStatus.IN_PROGRESS;
    ++this.request;
    this.setState(state => {
      const filteredCount = Math.max(0, state.response.filteredCount - 1);
      return {response: {...state.response, filteredCount,
        status: ScheduledReportsModel.ResponseStatus.READY,
        isEmpty: filteredCount === 0 && state.submission.filters.query === '',
        schedules: state.response.schedules.filter(entry => entry.id !== id)}};
    });
    this.deletions.set(id, this.deleteReport(schedule));
  };

  private mounted: boolean;
  private request: number;
  private generation: number;
  private loadingModel: Promise<void>;
  private deletions: Map<string, Promise<void>>;
  private deletionOrder: string[];
  private refreshRequired: boolean;
}
