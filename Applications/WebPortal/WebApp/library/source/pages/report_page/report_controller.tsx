import * as React from 'react';
import { SortableTableHeaderCell } from '../../components';
import { DateRange } from '../../models/date_range';
import { GeneratedReportsController, GeneratedReportsModel,
  GeneratedReportsPage, ReportTable } from './generated_reports_page';
import { ReportModel } from './report_model';
import { ScheduledReportsController, ScheduledReportsModel,
  ScheduledReportsPage } from './scheduled_reports_page';

interface Properties {

  /** The model providing the reporting subpage models. */
  model: ReportModel;

  /** The subpage to display. Defaults to SCHEDULED. */
  page?: ReportController.Page;

  /** Called to navigate to the Create Report Page. */
  onNewReport?: () => void;

  /** Called when a report action fails. */
  onActionError?: (error: unknown) => void;
}

interface State {
  model: ReportModel;
  scheduledReportsModel: ScheduledReportsModel;
  generatedReportsModel: GeneratedReportsModel;
  status: ScheduledReportsModel.ResponseStatus;
}

/** Loads the report model and composes the reporting subpage controllers. */
export class ReportController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      model: props.model,
      scheduledReportsModel: null,
      generatedReportsModel: null,
      status: ScheduledReportsModel.ResponseStatus.IN_PROGRESS
    };
    this.mounted = false;
    this.generation = 0;
  }

  public render(): JSX.Element {
    if(this.props.page === ReportController.Page.GENERATED) {
      if(this.state.model === this.props.model &&
          this.state.generatedReportsModel) {
        return <GeneratedReportsController
          model={this.state.generatedReportsModel}
          onNewReport={this.props.onNewReport}
          onActionError={this.props.onActionError}/>;
      }
      const status = (() => {
        if(this.state.model === this.props.model && this.state.status ===
            ScheduledReportsModel.ResponseStatus.ERROR) {
          return GeneratedReportsModel.ResponseStatus.ERROR;
        }
        return GeneratedReportsModel.ResponseStatus.IN_PROGRESS;
      })();
      return <GeneratedReportsPage
        filters={{query: '', dateRange: new DateRange(null, null)}}
        sort={{column: ReportTable.Column.DATE_CREATED,
          order: SortableTableHeaderCell.SortOrder.NONE}}
        pageIndex={0} recipientModel={null}
        response={{status, isEmpty: false, filteredCount: 0, reports: []}}
        onSubmit={this.onRetry} onNewReport={this.props.onNewReport}/>;
    }
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
      generatedReportsModel: null,
      status: ScheduledReportsModel.ResponseStatus.IN_PROGRESS});
    try {
      await model.load();
      if(this.mounted && generation === this.generation) {
        this.setState({scheduledReportsModel: model.scheduledReportsModel,
          generatedReportsModel: model.generatedReportsModel,
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

export namespace ReportController {

  /** The implemented reporting subpages. */
  export enum Page {

    /** Previously generated reports. */
    GENERATED,

    /** Reports scheduled for future generation. */
    SCHEDULED
  }
}
