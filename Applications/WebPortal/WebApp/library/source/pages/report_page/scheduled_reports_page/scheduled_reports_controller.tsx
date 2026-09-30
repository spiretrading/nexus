import * as React from 'react';
import { ScheduledReportsModel } from './scheduled_reports_model';
import { ScheduledReportsPage } from './scheduled_reports_page';

interface Properties {

  /** The model providing scheduled reports and their actions. */
  model: ScheduledReportsModel;

  /** Called to navigate to the Create Report Page. */
  onNewReport?: () => void;

  /** Called when running, duplicating, or deleting a report fails. */
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
    this.submit(this.state.submission);
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.model !== this.props.model) {
      ++this.generation;
      this.loadingModel = null;
      this.submit({filters: {query: ''}, pageIndex: 0});
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

  private async submit(submission: ScheduledReportsModel.Submission):
      Promise<void> {
    const request = ++this.request;
    const model = this.props.model;
    this.setState({submission, response: {
      ...this.state.response,
      status: ScheduledReportsModel.ResponseStatus.IN_PROGRESS
    }});
    try {
      await this.loadModel();
      if(!this.isCurrent(request, model)) {
        return;
      }
      const response = await model.loadSchedules(submission);
      if(!this.isCurrent(request, model)) {
        return;
      }
      const lastPage = Math.max(0, Math.ceil(
        response.filteredCount / ScheduledReportsModel.PAGE_SIZE) - 1);
      if(response.status === ScheduledReportsModel.ResponseStatus.READY &&
          submission.pageIndex > lastPage) {
        await this.submit({...submission, pageIndex: lastPage});
        return;
      }
      this.setState({response});
    } catch {
      if(this.isCurrent(request, model)) {
        this.setState({response: {
          status: ScheduledReportsModel.ResponseStatus.ERROR,
          isEmpty: false, filteredCount: 0, schedules: []
        }});
      }
    }
  }

  private isCurrent(request: number, model: ScheduledReportsModel): boolean {
    return this.mounted && request === this.request &&
      model === this.props.model;
  }

  private async execute(action: (model: ScheduledReportsModel) =>
      Promise<unknown>): Promise<void> {
    const model = this.props.model;
    const generation = this.generation;
    try {
      await this.loadModel();
      if(!this.mounted || generation !== this.generation) {
        return;
      }
      await action(model);
      if(this.mounted && generation === this.generation) {
        await this.submit(this.state.submission);
      }
    } catch(error) {
      if(this.mounted && generation === this.generation) {
        this.props.onActionError?.(error);
      }
    }
  }

  private onSubmit = (submission: ScheduledReportsModel.Submission) => {
    this.submit(submission);
  };

  private onRun = (id: string) => {
    this.execute(model => model.run(id));
  };

  private onDuplicate = (id: string) => {
    this.execute(model => model.duplicate(id));
  };

  private onDelete = (id: string) => {
    this.execute(model => model.delete(id));
  };

  private mounted: boolean;
  private request: number;
  private generation: number;
  private loadingModel: Promise<void>;
}
