import * as React from 'react';
import { PageLayout, SortableTableHeaderCell } from '../../components';
import { DateRange } from '../../models/date_range';
import { CreateReportController, CreateReportModel } from './create_report_page';
import { EditScheduledReportController, EditScheduledReportModel } from
  './edit_scheduled_report_page';
import { GeneratedReportsController, GeneratedReportsModel,
  GeneratedReportsPage, ReportTable } from './generated_reports_page';
import { ActivityTable, ReportActivityController, ReportActivityModel,
  ReportActivityPage } from './report_activity_page';
import { ReportModel } from './report_model';
import { ReportPage } from './report_page';
import { ReportDetailController, ReportDetailModel } from
  './report_detail_page';
import { ScheduledReportsController, ScheduledReportsModel,
  ScheduledReportsPage } from './scheduled_reports_page';

interface Properties {

  /** The model providing the reporting subpage models. */
  model: ReportModel;

  /** The subpage to display. Defaults to GENERATED. */
  page?: ReportController.Page;

  /** The prefix for reporting tab URLs. Defaults to an empty string. */
  basePath?: string;

  /** The schedule identifier when displaying EDIT_SCHEDULED. */
  scheduleId?: string;

  /** The generated report identifier when displaying DETAIL. */
  reportId?: string;

  /** Renders generated report content. Omit for download-only detail pages. */
  renderReportContent?: (report: ReportDetailModel.Report) => React.ReactNode;

  /** Called to navigate to the Create Report Page. */
  onNewReport?: () => void;

  /** Called to navigate after creating, scheduling, or editing a report. */
  onNavigate?: (page: ReportController.Page) => void;

  /** Called when a report action fails. */
  onActionError?: (error: unknown) => void;
}

interface State {
  model: ReportModel;
  scheduledReportsModel: ScheduledReportsModel;
  generatedReportsModel: GeneratedReportsModel;
  reportActivityModel: ReportActivityModel;
  createReportModel: CreateReportModel;
  editScheduledReportModel: EditScheduledReportModel;
  reportDetailModel: ReportDetailModel;
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
      reportActivityModel: null,
      createReportModel: null,
      editScheduledReportModel: null,
      reportDetailModel: null,
      status: ScheduledReportsModel.ResponseStatus.IN_PROGRESS
    };
    this.mounted = false;
    this.generation = 0;
  }

  public render(): JSX.Element {
    return <ReportPage current={getCurrentTab(this.props.page)}
        basePath={this.props.basePath} onNavigate={this.onTabNavigate}>
      {this.renderContent()}
    </ReportPage>;
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

  private renderContent(): JSX.Element {
    const page = this.props.page ?? ReportController.Page.GENERATED;
    if(page === ReportController.Page.DETAIL) {
      if(this.state.model === this.props.model &&
          this.state.reportDetailModel) {
        return <ReportDetailController model={this.state.reportDetailModel}
          id={this.props.reportId}
          renderContent={this.props.renderReportContent}
          onError={this.props.onActionError}/>;
      }
      return <PageLayout/>;
    }
    if(page === ReportController.Page.EDIT_SCHEDULED) {
      if(this.state.model === this.props.model &&
          this.state.editScheduledReportModel) {
        return <EditScheduledReportController
          model={this.state.editScheduledReportModel} id={this.props.scheduleId}
          onSaved={this.onSaved} onError={this.props.onActionError}/>;
      }
      return <PageLayout/>;
    }
    if(page === ReportController.Page.CREATE) {
      if(this.state.model === this.props.model && this.state.createReportModel) {
        return <CreateReportController model={this.state.createReportModel}
          onCreated={this.onCreated} onError={this.props.onActionError}/>;
      }
      return <PageLayout/>;
    }
    if(page === ReportController.Page.ACTIVITY) {
      if(this.state.model === this.props.model &&
          this.state.reportActivityModel) {
        return <ReportActivityController model={this.state.reportActivityModel}
          onNewReport={this.props.onNewReport}
          onActionError={this.props.onActionError}/>;
      }
      const status = (() => {
        if(this.state.model === this.props.model && this.state.status ===
            ScheduledReportsModel.ResponseStatus.ERROR) {
          return ReportActivityModel.ResponseStatus.ERROR;
        }
        return ReportActivityModel.ResponseStatus.IN_PROGRESS;
      })();
      return <ReportActivityPage
        sort={{column: ActivityTable.Column.DATE_MODIFIED,
          order: SortableTableHeaderCell.SortOrder.NONE}} pageIndex={0}
        response={{status, isEmpty: false, totalCount: 0, activities: []}}
        onSubmit={this.onRetry} onNewReport={this.props.onNewReport}/>;
    }
    if(page === ReportController.Page.GENERATED) {
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

  private async load(): Promise<void> {
    const model = this.props.model;
    const generation = ++this.generation;
    this.setState({model, scheduledReportsModel: null,
      generatedReportsModel: null,
      reportActivityModel: null,
      createReportModel: null,
      editScheduledReportModel: null,
      reportDetailModel: null,
      status: ScheduledReportsModel.ResponseStatus.IN_PROGRESS});
    try {
      await model.load();
      if(this.mounted && generation === this.generation) {
        this.setState({scheduledReportsModel: model.scheduledReportsModel,
          generatedReportsModel: model.generatedReportsModel,
          reportActivityModel: model.reportActivityModel,
          createReportModel: model.createReportModel,
          editScheduledReportModel: model.editScheduledReportModel,
          reportDetailModel: model.reportDetailModel,
          status: ScheduledReportsModel.ResponseStatus.READY});
      }
    } catch(error) {
      if(this.mounted && generation === this.generation) {
        this.setState({status: ScheduledReportsModel.ResponseStatus.ERROR});
        if(this.props.page === ReportController.Page.CREATE ||
            this.props.page === ReportController.Page.EDIT_SCHEDULED ||
            this.props.page === ReportController.Page.DETAIL) {
          this.props.onActionError?.(error);
        }
      }
    }
  }

  private onTabNavigate = (tab: ReportPage.Tab) => {
    const page = (() => {
      switch(tab) {
        case ReportPage.Tab.GENERATED:
          return ReportController.Page.GENERATED;
        case ReportPage.Tab.ACTIVITY:
          return ReportController.Page.ACTIVITY;
        case ReportPage.Tab.SCHEDULES:
          return ReportController.Page.SCHEDULED;
      }
    })();
    this.props.onNavigate?.(page);
  };

  private onCreated = (scheduled: boolean) => {
    if(scheduled) {
      this.props.onNavigate?.(ReportController.Page.SCHEDULED);
    } else {
      this.props.onNavigate?.(ReportController.Page.ACTIVITY);
    }
  };

  private onSaved = () => {
    this.props.onNavigate?.(ReportController.Page.SCHEDULED);
  };

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
    SCHEDULED,

    /** Report jobs being generated or awaiting retry. */
    ACTIVITY,

    /** Generate a report or create a schedule. */
    CREATE,

    /** Edit a previously scheduled report. */
    EDIT_SCHEDULED,

    /** View and download a generated report. */
    DETAIL
  }
}

function getCurrentTab(page: ReportController.Page): ReportPage.Tab {
  switch(page) {
    case ReportController.Page.ACTIVITY:
      return ReportPage.Tab.ACTIVITY;
    case ReportController.Page.SCHEDULED:
    case ReportController.Page.EDIT_SCHEDULED:
      return ReportPage.Tab.SCHEDULES;
    case ReportController.Page.CREATE:
      return null;
    default:
      return ReportPage.Tab.GENERATED;
  }
}
