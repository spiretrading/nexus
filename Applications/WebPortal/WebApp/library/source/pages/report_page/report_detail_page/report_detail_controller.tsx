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
}

/** Loads a generated report and displays its supplied content viewer. */
export class ReportDetailController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {model: props.model, id: props.id, report: null};
    this.mounted = false;
    this.generation = 0;
  }

  public render(): JSX.Element {
    if(!this.state.report || this.state.model !== this.props.model ||
        this.state.id !== this.props.id) {
      return <PageLayout/>;
    }
    const report = this.state.report;
    return <ReportDetailPage key={this.generation} title={report.title}
      parameters={report.parameters} filePath={report.filePath}
      content={this.props.renderContent?.(report)}/>;
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
  }

  private async load(): Promise<void> {
    const model = this.props.model;
    const id = this.props.id;
    const generation = ++this.generation;
    this.setState({model, id, report: null});
    try {
      await model.load();
      const report = await model.loadReport(id);
      if(this.mounted && generation === this.generation) {
        this.setState({report});
      }
    } catch(error) {
      if(this.mounted && generation === this.generation) {
        this.props.onError?.(error);
      }
    }
  }

  private mounted: boolean;
  private generation: number;
}
