import * as React from 'react';
import { ReportModel } from './report_model';
import { ScheduledReportsController, ScheduledReportsModel,
  ScheduledReportsPage } from './scheduled_reports_page';

interface Properties {

  /** The model providing the reporting subpage models. */
  model: ReportModel;

  /** Called to navigate to the Create Report Page. */
  onNewReport?: () => void;

  /** Called when a scheduled report action fails. */
  onActionError?: (error: unknown) => void;
}

interface State {
  model: ReportModel;
  scheduledReportsModel: ScheduledReportsModel;
  status: ScheduledReportsModel.ResponseStatus;
}

/** Loads the report model and composes the reporting subpage controllers. */
export class ReportController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      model: props.model,
      scheduledReportsModel: null,
      status: ScheduledReportsModel.ResponseStatus.IN_PROGRESS
    };
    this.mounted = false;
    this.generation = 0;
  }

  public render(): JSX.Element {
    if(this.state.model === this.props.model &&
        this.state.scheduledReportsModel) {
      return <ScheduledReportsController
        model={this.state.scheduledReportsModel}
        onNewReport={this.props.onNewReport}
        onActionError={this.props.onActionError}/>;
    }
    const status = (() => {
      if(this.state.model === this.props.model) {
        return this.state.status;
      }
      return ScheduledReportsModel.ResponseStatus.IN_PROGRESS;
    })();
    return <ScheduledReportsPage filters={{query: ''}} pageIndex={0}
      response={{status, isEmpty: false, filteredCount: 0, schedules: []}}
      onSubmit={this.onRetry} onNewReport={this.props.onNewReport}/>;
  }

  public componentDidMount(): void {
    this.mounted = true;
    this.load();
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.model !== this.props.model) {
      this.load();
    }
  }

  public componentWillUnmount(): void {
    this.mounted = false;
    ++this.generation;
  }

  private async load(): Promise<void> {
    const model = this.props.model;
    const generation = ++this.generation;
    this.setState({model, scheduledReportsModel: null,
      status: ScheduledReportsModel.ResponseStatus.IN_PROGRESS});
    try {
      await model.load();
      if(this.mounted && generation === this.generation) {
        this.setState({scheduledReportsModel: model.scheduledReportsModel,
          status: ScheduledReportsModel.ResponseStatus.READY});
      }
    } catch {
      if(this.mounted && generation === this.generation) {
        this.setState({status: ScheduledReportsModel.ResponseStatus.ERROR});
      }
    }
  }

  private onRetry = () => {
    this.load();
  };

  private mounted: boolean;
  private generation: number;
}
