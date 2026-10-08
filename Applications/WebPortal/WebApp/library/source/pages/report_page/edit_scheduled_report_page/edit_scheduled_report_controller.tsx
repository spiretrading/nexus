import * as Beam from 'beam';
import * as React from 'react';
import { PageLayout } from '../../../components';
import { DateRange, isDateRangeEqual, isDateRuleEqual, isScopeEqual } from
  '../../../models';
import { ReportFormTemplate } from '../report_form_template';
import { EditScheduledReportModel } from './edit_scheduled_report_model';
import { EditScheduledReportPage } from './edit_scheduled_report_page';

interface Properties {

  /** The model supplying scheduled configurations and saving changes. */
  model: EditScheduledReportModel;

  /** The identifier of the schedule to edit. */
  id: string;

  /** Called after successfully saving the schedule. */
  onSaved?: () => void;

  /** Reports a load or submission failure to the host application. */
  onError?: (error: unknown) => void;
}

interface State {
  model: EditScheduledReportModel;
  loaded: boolean;
  submitting: boolean;
  errorMessage: string;
  id: string;
  original: ReportFormTemplate.Value;
  value: ReportFormTemplate.Value;
}

/** Loads an existing schedule and saves changes before navigating back. */
export class EditScheduledReportController extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {model: props.model, id: props.id, loaded: false,
      submitting: false, original: null, value: null, errorMessage: ''};
    this.mounted = false;
    this.generation = 0;
    this.submitting = false;
  }

  public render(): JSX.Element {
    if(!this.state.loaded || this.state.model !== this.props.model ||
        this.state.id !== this.props.id) {
      return <PageLayout/>;
    }
    return <EditScheduledReportPage key={this.generation}
      reports={this.props.model.reports} value={this.state.value}
      accountModel={this.props.model.accountModel}
      scopeModel={this.props.model.scopeModel} onChange={this.onChange}
      submitDisabled={this.state.submitting ||
        isEqual(this.state.value, this.state.original)}
      errorMessage={this.state.errorMessage} onSubmit={this.onSubmit}/>;
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
    this.submitting = false;
    this.setState({model, id, loaded: false, submitting: false, original: null,
      value: null, errorMessage: ''});
    try {
      await model.load();
      const value = await model.loadReport(id);
      if(!this.mounted || generation !== this.generation) {
        return;
      }
      this.setState({loaded: true, original: value, value});
    } catch(error) {
      if(this.mounted && generation === this.generation) {
        this.props.onError?.(error);
      }
    }
  }

  private onChange = (value: ReportFormTemplate.Value) => {
    this.setState({value, errorMessage: ''});
  };

  private onSubmit = async (value: ReportFormTemplate.Value) => {
    if(this.submitting || isEqual(value, this.state.original)) {
      return;
    }
    this.submitting = true;
    this.setState({submitting: true, errorMessage: ''});
    const model = this.props.model;
    const generation = this.generation;
    const id = this.props.id;
    try {
      await model.submit(id, value);
      if(this.mounted && generation === this.generation) {
        this.props.onSaved?.();
      }
    } catch(error) {
      if(this.mounted && generation === this.generation) {
        this.setState({errorMessage: 'Server issue'});
        this.props.onError?.(error);
      }
    } finally {
      if(this.mounted && generation === this.generation) {
        this.submitting = false;
        this.setState({submitting: false});
      }
    }
  };

  private mounted: boolean;
  private generation: number;
  private submitting: boolean;
}

function isEqual(left: ReportFormTemplate.Value,
    right: ReportFormTemplate.Value): boolean {
  const equals = (a: any, b: any): boolean => {
    if(a === b) {
      return true;
    } else if(a == null || b == null) {
      return false;
    } else if(a instanceof DateRange && b instanceof DateRange) {
      return isDateRangeEqual(a, b);
    } else if(typeof a.isGlobal === 'boolean' &&
        typeof b.isGlobal === 'boolean') {
      return isScopeEqual(a, b);
    } else if(Array.isArray(a) && Array.isArray(b)) {
      return a.length === b.length &&
        a.every((entry, i) => equals(entry, b[i]));
    }
    return Beam.equals(a, b);
  };
  return left.reportType === right.reportType &&
    left.scheduled === right.scheduled && left.repeats === right.repeats &&
    equals(left.recipients, right.recipients) &&
    equals(left.scheduleDateTime, right.scheduleDateTime) &&
    left.repeatInterval?.count === right.repeatInterval?.count &&
    left.repeatInterval?.unit === right.repeatInterval?.unit &&
    ((!left.repeatRule && !right.repeatRule) ||
      !!left.repeatRule && !!right.repeatRule &&
      isDateRuleEqual(left.repeatRule, right.repeatRule)) &&
    Object.keys(left.parameters).length ===
      Object.keys(right.parameters).length &&
    Object.keys(left.parameters).every(name =>
      equals(left.parameters[name], right.parameters[name]));
}
