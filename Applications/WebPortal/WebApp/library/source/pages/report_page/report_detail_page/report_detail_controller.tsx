import * as React from 'react';
import { PageLayout } from '../../../components';
import { ReportDetailModel } from './report_detail_model';
import { ReportDetailPage } from './report_detail_page';

interface Properties {

  /** The model supplying generated reports. */
  model: ReportDetailModel;

  /** The identifier of the report to display. */
  id: string;

  /** Renders the report content. Omit for a download-only page. */
  renderContent?: (report: ReportDetailModel.Report) => React.ReactNode;

  /** Reports retrieval failures to the host application. */
  onError?: (error: unknown) => void;
}

interface State {
  model: ReportDetailModel;
  id: string;
  report: ReportDetailModel.Report;
  status: ReportDetailPage.Status;
}

/** Loads a generated report and displays its supplied content viewer. */
export class ReportDetailController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {model: props.model, id: props.id, report: null, status: null};
    this.mounted = false;
    this.generation = 0;
    this.loading = false;
    this.timer = null;
  }

  public render(): JSX.Element {
    if(this.state.status === null || this.state.model !== this.props.model ||
        this.state.id !== this.props.id) {
      return <PageLayout/>;
    }
    const report = this.state.report;
    return <ReportDetailPage title={report?.title ?? ''}
      parameters={report?.parameters ?? []} filePath={report?.filePath}
      status={this.state.status} onRetry={this.onRetry}
      content={report && this.props.renderContent?.(report)}/>;
  }

  public componentDidMount(): void {
    this.mounted = true;
    this.load();
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.model !== this.props.model || previous.id !== this.props.id) {
      this.load();
    }
  }

  public componentWillUnmount(): void {
    this.mounted = false;
    ++this.generation;
    window.clearTimeout(this.timer);
  }

  private async load(): Promise<void> {
    const model = this.props.model;
    const id = this.props.id;
    const generation = ++this.generation;
    this.loading = true;
    window.clearTimeout(this.timer);
    const status = (() => {
      if(this.state.model === model && this.state.id === id) {
        return this.state.status;
      }
      return null;
    })();
    this.setState({model, id, report: null, status});
    const PLACEHOLDER_DELAY = 500;
    this.timer = window.setTimeout(() => {
      if(this.mounted && generation === this.generation) {
        this.setState({status: ReportDetailPage.Status.IN_PROGRESS});
      }
    }, PLACEHOLDER_DELAY);
    try {
      await model.load();
      if(!this.mounted || generation !== this.generation) {
        return;
      }
      const report = await model.loadReport(id);
      if(this.mounted && generation === this.generation) {
        this.setState({report, status: ReportDetailPage.Status.READY});
      }
    } catch(error) {
      if(this.mounted && generation === this.generation) {
        this.setState({status: ReportDetailPage.Status.ERROR});
        this.props.onError?.(error);
      }
    } finally {
      if(this.mounted && generation === this.generation) {
        this.loading = false;
        window.clearTimeout(this.timer);
        this.timer = null;
      }
    }
  }

  private onRetry = () => {
    if(!this.loading) {
      this.load();
    }
  };

  private mounted: boolean;
  private generation: number;
  private loading: boolean;
  private timer: number;
}
