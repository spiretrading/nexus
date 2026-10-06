import * as React from 'react';
import { SortableTableHeaderCell } from '../../../components';
import { ActivityTable } from './activity_table';
import { ReportActivityModel } from './report_activity_model';
import { ReportActivityPage } from './report_activity_page';

interface Properties {

  /** The model providing report jobs and their actions. */
  model: ReportActivityModel;

  /** Navigates to the Create Report Page. */
  onNewReport?: () => void;

  /** Reports an action failure or failed background refresh. */
  onActionError?: (error: unknown) => void;
}

interface State {
  submission: ReportActivityModel.Submission;
  response: ReportActivityModel.Response;
}

/** Loads report activity and applies cancellation and retry actions. */
export class ReportActivityController extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {submission: makeSubmission(),
      response: {status: ReportActivityModel.ResponseStatus.IN_PROGRESS,
        isEmpty: false, totalCount: 0, activities: []}};
    this.mounted = false;
    this.request = 0;
    this.generation = 0;
    this.loadingModel = null;
    this.mutations = Promise.resolve();
    this.cancellationOrder = [];
    this.pendingActions = 0;
    this.timer = null;
  }

  public render(): JSX.Element {
    return <ReportActivityPage key={this.generation}
      {...this.state.submission} response={this.state.response}
      onSubmit={this.onSubmit}
      onNewReport={this.props.onNewReport} onCancel={this.onCancel}
      onRetry={this.onRetry}/>;
  }

  public componentDidMount(): void {
    this.mounted = true;
    this.submit(this.state.submission, false);
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.model !== this.props.model) {
      this.clearRefresh();
      ++this.generation;
      this.loadingModel = null;
      this.mutations = Promise.resolve();
      this.cancellationOrder = [];
      this.pendingActions = 0;
      this.setState({response: {
        status: ReportActivityModel.ResponseStatus.IN_PROGRESS,
          isEmpty: false, totalCount: 0, activities: []}}, () =>
        this.submit(makeSubmission(), false));
    }
  }

  public componentWillUnmount(): void {
    this.clearRefresh();
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

  private async submit(submission: ReportActivityModel.Submission,
      background: boolean): Promise<void> {
    this.clearRefresh();
    const request = ++this.request;
    const model = this.props.model;
    if(!background) {
      this.setState({submission, response: {...this.state.response,
        status: ReportActivityModel.ResponseStatus.IN_PROGRESS}});
    }
    try {
      await this.loadModel();
      await this.mutations;
      if(!this.isCurrent(request, model)) {
        return;
      }
      const response = await model.loadActivities(submission);
      if(!this.isCurrent(request, model)) {
        return;
      }
      if(background &&
          response.status === ReportActivityModel.ResponseStatus.ERROR) {
        this.props.onActionError?.(
          new Error('Report activity request failed.'));
        this.scheduleRefresh();
        return;
      }
      const lastPage = Math.max(0, Math.ceil(
        response.totalCount / ReportActivityModel.PAGE_SIZE) - 1);
      if(response.status === ReportActivityModel.ResponseStatus.READY &&
          submission.pageIndex > lastPage) {
        await this.submit({...submission, pageIndex: lastPage}, false);
        return;
      }
      this.setState({response});
      if(response.status !== ReportActivityModel.ResponseStatus.ERROR) {
        this.scheduleRefresh();
      }
    } catch(error) {
      if(this.isCurrent(request, model)) {
        if(background) {
          this.props.onActionError?.(error);
          this.scheduleRefresh();
        } else {
          this.setState({response: {
            status: ReportActivityModel.ResponseStatus.ERROR,
            isEmpty: false, totalCount: 0, activities: []}});
        }
      }
    }
  }

  private isCurrent(request: number, model: ReportActivityModel): boolean {
    return this.mounted && request === this.request &&
      model === this.props.model;
  }

  private clearRefresh(): void {
    if(this.timer !== null) {
      window.clearTimeout(this.timer);
      this.timer = null;
    }
  }

  private scheduleRefresh(): void {
    this.clearRefresh();
    if(this.mounted && this.pendingActions === 0) {
      const REFRESH_INTERVAL = 5000;
      this.timer = window.setTimeout(this.onRefresh, REFRESH_INTERVAL);
    }
  }

  private onSubmit = (submission: ReportActivityModel.Submission) => {
    this.submit(submission, false);
  };

  private onCancel = (ids: readonly string[]) => {
    const selected = new Set(ids);
    const original = this.state.response;
    const removed = original.activities.filter(entry => selected.has(entry.id));
    if(removed.length === 0) {
      return;
    }
    this.clearRefresh();
    if(this.pendingActions === 0) {
      this.cancellationOrder = original.activities.map(report => report.id);
    }
    ++this.pendingActions;
    const order = this.cancellationOrder;
    const model = this.props.model;
    const generation = this.generation;
    const submission = this.state.submission;
    const totalCount = Math.max(0, original.totalCount - removed.length);
    const optimistic = {...original, totalCount,
      isEmpty: totalCount === 0,
      activities: original.activities.filter(entry => !selected.has(entry.id))};
    ++this.request;
    this.setState({response: optimistic});
    const loading = this.loadModel();
    this.mutations = this.mutations.then(async () => {
      try {
        await loading;
        await model.cancel(removed.map(report => report.id));
      } catch(error) {
        if(this.mounted && generation === this.generation) {
          this.setState(state => {
            if(state.submission !== submission || state.response.status !==
                ReportActivityModel.ResponseStatus.READY) {
              return null;
            }
            const activities = [...state.response.activities];
            let restored = 0;
            for(const report of removed) {
              if(activities.some(entry => entry.id === report.id)) {
                continue;
              }
              const position = order.indexOf(report.id);
              const index = activities.findIndex(entry =>
                order.indexOf(entry.id) > position);
              if(index < 0) {
                activities.push(report);
              } else {
                activities.splice(index, 0, report);
              }
              ++restored;
            }
            return {response: {...state.response, activities, isEmpty: false,
              totalCount: state.response.totalCount + restored}};
          });
          this.props.onActionError?.(error);
        }
      }
    });
    this.mutations.then(() => {
      if(this.mounted && generation === this.generation) {
        --this.pendingActions;
        if(this.pendingActions === 0) {
          this.submit(this.state.submission, this.state.response.status ===
            ReportActivityModel.ResponseStatus.READY);
        }
      }
    });
  };

  private onRetry = (ids: readonly string[]) => {
    this.clearRefresh();
    const model = this.props.model;
    const generation = this.generation;
    const loading = this.loadModel();
    const selected = [...ids];
    if(this.pendingActions === 0) {
      this.cancellationOrder = this.state.response.activities.
        map(entry => entry.id);
    }
    ++this.request;
    ++this.pendingActions;
    this.mutations = this.mutations.then(async () => {
      try {
        await loading;
        await model.retry(selected);
      } catch(error) {
        if(this.mounted && generation === this.generation) {
          this.props.onActionError?.(error);
        }
      }
    });
    this.mutations.then(() => {
      if(this.mounted && generation === this.generation) {
        --this.pendingActions;
        if(this.pendingActions === 0) {
          this.submit(this.state.submission, this.state.response.status ===
            ReportActivityModel.ResponseStatus.READY);
        }
      }
    });
  };

  private onRefresh = () => {
    this.timer = null;
    this.submit(this.state.submission, true);
  };

  private mounted: boolean;
  private request: number;
  private generation: number;
  private loadingModel: Promise<void>;
  private mutations: Promise<void>;
  private cancellationOrder: string[];
  private pendingActions: number;
  private timer: number;
}

function makeSubmission(): ReportActivityModel.Submission {
  return {sort: {column: ActivityTable.Column.DATE_MODIFIED,
      order: SortableTableHeaderCell.SortOrder.NONE}, pageIndex: 0};
}
