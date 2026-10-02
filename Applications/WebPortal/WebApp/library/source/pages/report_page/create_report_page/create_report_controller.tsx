import * as React from 'react';
import { PageLayout } from '../../../components';
import { Interval } from '../../../models';
import { ReportFormTemplate } from '../report_form_template';
import { CreateReportModel } from './create_report_model';
import { CreateReportPage } from './create_report_page';

interface Properties {

  /** The model supplying report types, input models, and submission. */
  model: CreateReportModel;

  /** Called after successful creation with the destination and identifier. */
  onCreated?: (scheduled: boolean, id: string) => void;

  /** Reports a load or submission failure to the host application. */
  onError?: (error: unknown) => void;
}

interface State {
  model: CreateReportModel;
  loaded: boolean;
  value: ReportFormTemplate.Value;
}

/** Loads the creation form and submits it before requesting navigation. */
export class CreateReportController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {model: props.model, loaded: false, value: null};
    this.mounted = false;
    this.generation = 0;
    this.submitting = false;
  }

  public render(): JSX.Element {
    if(!this.state.loaded || this.state.model !== this.props.model) {
      return <PageLayout/>;
    }
    return <CreateReportPage key={this.generation}
      reports={this.props.model.reports} value={this.state.value}
      accountModel={this.props.model.accountModel}
      scopeModel={this.props.model.scopeModel} onChange={this.onChange}
      onSubmit={this.onSubmit}/>;
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
    this.submitting = false;
    this.setState({model, loaded: false, value: null});
    try {
      await model.load();
      if(!this.mounted || generation !== this.generation) {
        return;
      }
      const report = model.reports[0];
      const value = (() => {
        if(report) {
          return ReportFormTemplate.makeValue(report);
        }
        return {reportType: '', parameters: {}, recipients: [],
          scheduled: false, scheduleDateTime: null, repeats: false,
          repeatInterval: new Interval(null, Interval.Unit.DAY)};
      })();
      this.setState({loaded: true, value});
    } catch(error) {
      if(this.mounted && generation === this.generation) {
        this.props.onError?.(error);
      }
    }
  }

  private onChange = (value: ReportFormTemplate.Value) => {
    this.setState({value});
  };

  private onSubmit = async (value: ReportFormTemplate.Value) => {
    if(this.submitting) {
      return;
    }
    this.submitting = true;
    const model = this.props.model;
    const generation = this.generation;
    const scheduled = value.scheduled;
    try {
      const id = await model.submit(value);
      if(this.mounted && generation === this.generation) {
        this.props.onCreated?.(scheduled, id);
      }
    } catch(error) {
      if(this.mounted && generation === this.generation) {
        this.props.onError?.(error);
      }
    } finally {
      if(this.mounted && generation === this.generation) {
        this.submitting = false;
      }
    }
  };

  private mounted: boolean;
  private generation: number;
  private submitting: boolean;
}
