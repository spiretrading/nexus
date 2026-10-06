import * as Beam from 'beam';
import * as React from 'react';
import { AccountGroupQueryModel, SortableTableHeaderCell } from
  '../../../components';
import { DateRange } from '../../../models/date_range';
import { GeneratedReportsModel } from './generated_reports_model';
import { GeneratedReportsPage } from './generated_reports_page';
import { ReportTable } from './report_table';

interface Properties {

  /** The model providing generated reports and their actions. */
  model: GeneratedReportsModel;

  /** Navigates to the Create Report Page. */
  onNewReport?: () => void;

  /** Reports an action failure or failed background refresh. */
  onActionError?: (error: unknown) => void;
}

interface State {
  submission: GeneratedReportsModel.Submission;
  response: GeneratedReportsModel.Response;
  recipientModel: AccountGroupQueryModel;
}

/** Loads generated reports and applies filtering and bulk actions. */
export class GeneratedReportsController extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {submission: makeSubmission(), recipientModel: null,
      response: {status: GeneratedReportsModel.ResponseStatus.IN_PROGRESS,
        isEmpty: false, filteredCount: 0, reports: []}};
    this.mounted = false;
    this.request = 0;
    this.generation = 0;
    this.loadingModel = null;
    this.mutations = Promise.resolve();
    this.deletionOrder = [];
    this.pendingDeletions = 0;
  }

  public render(): JSX.Element {
    return <GeneratedReportsPage key={this.generation}
      {...this.state.submission} response={this.state.response}
      recipientModel={this.state.recipientModel} onSubmit={this.onSubmit}
      onNewReport={this.props.onNewReport} onDelete={this.onDelete}
      onShare={this.onShare} onDownload={this.onDownload}/>;
  }

  public componentDidMount(): void {
    this.mounted = true;
    this.submit(this.state.submission, false);
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.model !== this.props.model) {
      ++this.generation;
      this.loadingModel = null;
      this.mutations = Promise.resolve();
      this.deletionOrder = [];
      this.pendingDeletions = 0;
      this.setState({recipientModel: null,
        response: {status: GeneratedReportsModel.ResponseStatus.IN_PROGRESS,
          isEmpty: false, filteredCount: 0, reports: []}}, () =>
        this.submit(makeSubmission(), false));
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

  private async submit(submission: GeneratedReportsModel.Submission,
      background: boolean): Promise<void> {
    const request = ++this.request;
    const model = this.props.model;
    if(!background) {
      this.setState({submission, response: {...this.state.response,
        status: GeneratedReportsModel.ResponseStatus.IN_PROGRESS}});
    }
    try {
      await this.loadModel();
      await this.mutations;
      if(!this.isCurrent(request, model)) {
        return;
      }
      this.setState({recipientModel: model.recipientModel});
      const response = await model.query(submission);
      if(!this.isCurrent(request, model)) {
        return;
      }
      if(background &&
          response.status === GeneratedReportsModel.ResponseStatus.ERROR) {
        this.props.onActionError?.(
          new Error('Generated reports request failed.'));
        return;
      }
      const lastPage = Math.max(0, Math.ceil(
        response.filteredCount / GeneratedReportsModel.PAGE_SIZE) - 1);
      if(response.status === GeneratedReportsModel.ResponseStatus.READY &&
          submission.pageIndex > lastPage) {
        await this.submit({...submission, pageIndex: lastPage}, false);
        return;
      }
      this.setState({response});
    } catch(error) {
      if(this.isCurrent(request, model)) {
        if(background) {
          this.props.onActionError?.(error);
        } else {
          this.setState({response: {
            status: GeneratedReportsModel.ResponseStatus.ERROR,
            isEmpty: false, filteredCount: 0, reports: []}});
        }
      }
    }
  }

  private isCurrent(request: number, model: GeneratedReportsModel): boolean {
    return this.mounted && request === this.request &&
      model === this.props.model;
  }

  private async execute(action:
      (model: GeneratedReportsModel) => Promise<void>): Promise<void> {
    const model = this.props.model;
    const generation = this.generation;
    try {
      await this.loadModel();
      await action(model);
    } catch(error) {
      if(this.mounted && generation === this.generation) {
        this.props.onActionError?.(error);
      }
    }
  }

  private onSubmit = (submission: GeneratedReportsModel.Submission) => {
    this.submit(submission, false);
  };

  private onDelete = (ids: readonly string[]) => {
    const selected = new Set(ids);
    const original = this.state.response;
    const removed = original.reports.filter(report => selected.has(report.id));
    if(removed.length === 0) {
      return;
    }
    if(this.pendingDeletions === 0) {
      this.deletionOrder = original.reports.map(report => report.id);
    }
    ++this.pendingDeletions;
    const order = this.deletionOrder;
    const model = this.props.model;
    const generation = this.generation;
    const submission = this.state.submission;
    const filteredCount = Math.max(0, original.filteredCount - removed.length);
    const filters = this.state.submission.filters;
    const optimistic = {...original, filteredCount,
      isEmpty: filteredCount === 0 && filters.query === '' &&
        !filters.dateRange.start && !filters.dateRange.end,
      reports: original.reports.filter(report => !selected.has(report.id))};
    ++this.request;
    this.setState({response: optimistic});
    const loading = this.loadModel();
    this.mutations = this.mutations.then(async () => {
      try {
        await loading;
        await model.delete(removed.map(report => report.id));
      } catch(error) {
        if(this.mounted && generation === this.generation) {
          this.setState(state => {
            if(state.submission !== submission || state.response.status !==
                GeneratedReportsModel.ResponseStatus.READY) {
              return null;
            }
            const reports = [...state.response.reports];
            let restored = 0;
            for(const report of removed) {
              if(reports.some(entry => entry.id === report.id)) {
                continue;
              }
              const position = order.indexOf(report.id);
              const index = reports.findIndex(entry =>
                order.indexOf(entry.id) > position);
              if(index < 0) {
                reports.push(report);
              } else {
                reports.splice(index, 0, report);
              }
              ++restored;
            }
            return {response: {...state.response, reports, isEmpty: false,
              filteredCount: state.response.filteredCount + restored}};
          });
          this.props.onActionError?.(error);
        }
      }
    });
    this.mutations.then(() => {
      if(this.mounted && generation === this.generation) {
        --this.pendingDeletions;
        if(this.pendingDeletions === 0) {
          this.submit(this.state.submission, this.state.response.status ===
            GeneratedReportsModel.ResponseStatus.READY);
        }
      }
    });
  };

  private onShare = (ids: readonly string[],
      recipients: readonly Beam.DirectoryEntry[]) => {
    this.execute(model => model.share(ids, recipients));
  };

  private onDownload = (ids: readonly string[]) => {
    this.execute(model => model.download(ids));
  };

  private mounted: boolean;
  private request: number;
  private generation: number;
  private loadingModel: Promise<void>;
  private mutations: Promise<void>;
  private deletionOrder: string[];
  private pendingDeletions: number;
}

function makeSubmission(): GeneratedReportsModel.Submission {
  return {filters: {query: '', dateRange: new DateRange(null, null)},
    sort: {column: ReportTable.Column.DATE_CREATED,
      order: SortableTableHeaderCell.SortOrder.NONE}, pageIndex: 0};
}
