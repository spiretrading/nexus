import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { AccountGroupQueryModel, Button, Checkbox, InputErrorMessage,
  InputGroup, IntervalInput, RadioButton } from '../../components';
import { Interval, QueryModel, ValidationError } from '../../models';
import { ReportDefinition } from './report_definition';
import { ReportParameterDefinition } from './report_parameter_definition';
import { ReportParameterInput } from './report_parameter_input';
import { parseReportParameterValue, ReportParameterValue,
  validateReportParameter } from './report_parameter_value';
import { ReportTypeSelect } from './report_type_select';

interface Properties {

  /** The form's heading. */
  title: string;

  /** The report types available to the current user. */
  reports: readonly ReportDefinition[];

  /** The current form values. */
  value: ReportFormTemplate.Value;

  /** The mode of the form. Defaults to CREATE. */
  mode?: ReportFormTemplate.Mode;

  /** Whether the Run fieldset is displayed. Defaults to true. */
  showRuntime?: boolean;

  /** The model used to look up accounts and groups. */
  accountModel: AccountGroupQueryModel;

  /** The model used to look up scope entries. */
  scopeModel: QueryModel<Nexus.Scope>;

  /** A fixed reference time for validation; otherwise the clock is used. */
  now?: Beam.DateTime;

  /** Reports edits to the form values. */
  onChange?: (value: ReportFormTemplate.Value) => void;

  /** Reports a validated form submission. */
  onSubmit?: (value: ReportFormTemplate.Value) => void;
}

interface State {
  errors: ReadonlyMap<string, ValidationError>;
  now: Beam.DateTime;
  sharingError: ValidationError;
  scheduleError: ValidationError;
  intervalError: ValidationError;
}

/** Edits report parameters, recipients, and optional scheduling. */
export class ReportFormTemplate extends React.Component<Properties, State> {

  /** Constructs the initial form values from a report's defaults. */
  public static makeValue(report: ReportDefinition): ReportFormTemplate.Value {
    const parameters: Record<string, ReportParameterValue> = {};
    for(const parameter of report.parameters) {
      parameters[parameter.name] =
        parseReportParameterValue(parameter.type, parameter.defaultValue);
    }
    return {reportType: report.id, parameters, recipients: [], scheduled: false,
      scheduleDateTime: null, repeats: false,
      repeatInterval: new Interval(null, Interval.Unit.DAY)};
  }

  constructor(props: Properties) {
    super(props);
    this.state = {errors: new Map(), now: Beam.DateTime.now(),
      sharingError: ValidationError.NONE, scheduleError: ValidationError.NONE,
      intervalError: ValidationError.NONE};
    this.identifier = `report-form-${ReportFormTemplate.nextIdentifier++}`;
    this.timer = null;
    this.mounted = false;
  }

  public render(): JSX.Element {
    const report = this.props.reports.find(entry =>
      entry.id === this.props.value.reportType);
    const props = this.props;
    return <div className={css(STYLES.container)}>
      <form noValidate className={css(STYLES.form)} onSubmit={this.onSubmit}>
        <h1 className={css(STYLES.heading)}>{props.title}</h1>
        <TypeFieldset reports={props.reports} value={props.value.reportType}
          parametersId={`${this.identifier}-parameters`}
          onChange={this.onReportChange}/>
        <ParametersFieldset id={`${this.identifier}-parameters`} report={report}
          value={props.value} accountModel={props.accountModel}
          scopeModel={props.scopeModel} onChange={this.onParameterChange}
          onValidation={this.onParameterValidation}/>
        <ShareFieldset value={props.value.recipients}
          accountModel={props.accountModel} scopeModel={props.scopeModel}
          onChange={this.onRecipientsChange}
          onValidation={this.onSharingValidation}/>
        <Collapse open={props.showRuntime !== false}>
          <RunFieldset name={`${this.identifier}-runtime`}
            scheduled={props.value.scheduled}
            scheduleId={`${this.identifier}-schedule`}
            onChange={this.onScheduledChange}/>
        </Collapse>
        <Collapse open={props.value.scheduled}>
          <ScheduleFieldset id={`${this.identifier}-schedule`}
            value={props.value} accountModel={props.accountModel}
            scopeModel={props.scopeModel} error={this.getDateTimeError()}
            onDateChange={this.onDateTimeChange}
            onValidation={this.onDateTimeValidation}
            onRepeatsChange={this.onRepeatsChange}
            onIntervalChange={this.onIntervalChange}
            intervalError={this.state.intervalError}
            onIntervalInput={this.onIntervalInput}/>
        </Collapse>
        <SubmitSection mode={props.mode} scheduled={props.value.scheduled}
          valid={this.isValid()}/>
      </form>
    </div>;
  }

  public componentDidMount(): void {
    this.mounted = true;
    const CLOCK_INTERVAL = 1000;
    this.timer = window.setInterval(this.onClock, CLOCK_INTERVAL);
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.value.reportType !== this.props.value.reportType) {
      this.setState({errors: new Map()});
    }
  }

  public componentWillUnmount(): void {
    this.mounted = false;
    window.clearInterval(this.timer);
  }

  private getDateTimeError(): ValidationError {
    const error = validateReportParameter(SCHEDULE_PARAMETER,
      this.props.value.scheduleDateTime);
    if(error !== ValidationError.NONE) {
      return error;
    }
    if(this.props.mode !== ReportFormTemplate.Mode.EDIT &&
        this.props.value.scheduleDateTime.compare(
          this.props.now ?? Beam.DateTime.now()) <= 0) {
      return ValidationError.PAST_DATE;
    }
    return ValidationError.NONE;
  }

  private isValid(): boolean {
    const {value} = this.props;
    const report = this.props.reports.find(entry =>
      entry.id === value.reportType);
    if(!report || report.parameters.some(parameter =>
        validateReportParameter(parameter, value.parameters[parameter.name]) !==
          ValidationError.NONE || this.state.errors.get(parameter.name)) ||
        validateReportParameter(SHARING_PARAMETER, value.recipients) !==
          ValidationError.NONE || this.state.sharingError) {
      return false;
    }
    if(value.scheduled && (this.getDateTimeError() !== ValidationError.NONE ||
        this.state.scheduleError)) {
      return false;
    }
    if(value.scheduled && value.repeats) {
      const interval = value.repeatInterval;
      if(this.state.intervalError || !interval ||
          !Number.isInteger(interval.count) || interval.count < 1) {
        return false;
      }
    }
    return true;
  }

  private update(value: Partial<ReportFormTemplate.Value>): void {
    this.props.onChange?.({...this.props.value, ...value});
  }

  private setError(name: string, error: ValidationError): void {
    if((this.state.errors.get(name) ?? ValidationError.NONE) !== error) {
      this.setState(state => {
        const errors = new Map(state.errors);
        errors.set(name, error);
        return {errors};
      });
    }
  }

  private onReportChange = (reportType: string) => {
    const report = this.props.reports.find(entry => entry.id === reportType);
    if(report) {
      this.setState({errors: new Map()});
      this.update({reportType,
        parameters: ReportFormTemplate.makeValue(report).parameters});
    }
  };

  private onParameterChange = (name: string, value: ReportParameterValue) => {
    this.update({parameters: {...this.props.value.parameters, [name]: value}});
  };

  private onParameterValidation = (name: string, error: ValidationError) => {
    this.setError(name, error);
  };

  private onSharingValidation = (error: ValidationError) => {
    if(error !== this.state.sharingError) {
      this.setState({sharingError: error});
    }
  };

  private onRecipientsChange = (value: ReportParameterValue) => {
    this.update({recipients: value as readonly Beam.DirectoryEntry[]});
  };

  private onScheduledChange = (scheduled: boolean) => {
    this.update({scheduled});
  };

  private onDateTimeChange = (value: ReportParameterValue) => {
    this.update({scheduleDateTime: value as Beam.DateTime});
  };

  private onDateTimeValidation = (error: ValidationError) => {
    if(error !== this.state.scheduleError) {
      this.setState({scheduleError: error});
    }
  };

  private onRepeatsChange = (repeats: boolean) => {
    this.update({repeats});
  };

  private onIntervalChange = (repeatInterval: Interval) => {
    this.setState({intervalError: ValidationError.NONE});
    this.update({repeatInterval});
  };

  private onIntervalInput = (event: React.FormEvent<HTMLDivElement>) => {
    if((event.target as HTMLElement).tagName !== 'INPUT') {
      return;
    }
    const text = event.currentTarget.querySelector('input').value;
    let intervalError = ValidationError.NONE;
    if(text === '') {
      intervalError = ValidationError.REQUIRED;
    } else if(!/^\d+$/.test(text) || Number(text) < 1) {
      intervalError = ValidationError.FORMAT;
    }
    this.setState({intervalError});
  };

  private onSubmit = (event: React.FormEvent) => {
    event.preventDefault();
    (document.activeElement as HTMLElement)?.blur();
    queueMicrotask(() => {
      if(this.mounted && this.isValid()) {
        this.props.onSubmit?.(this.props.value);
      }
    });
  };

  private onClock = () => {
    if(!this.props.now && this.props.value.scheduled) {
      this.setState({now: Beam.DateTime.now()});
    }
  };

  private static nextIdentifier = 0;
  private identifier: string;
  private timer: number;
  private mounted: boolean;
}

export namespace ReportFormTemplate {

  /** The purpose of the report form. */
  export enum Mode {

    /** Generate a report or create a schedule. */
    CREATE,

    /** Edit an existing scheduled report. */
    EDIT
  }

  /** The report configuration edited by the form. */
  export interface Value {

    /** The stable identifier of the selected report type. */
    reportType: string;

    /** The typed values keyed by parameter identifier. */
    parameters: Readonly<Record<string, ReportParameterValue>>;

    /** Accounts and groups receiving the report. */
    recipients: readonly Beam.DirectoryEntry[];

    /** Whether the report runs on a schedule. */
    scheduled: boolean;

    /** The requested start date and time. */
    scheduleDateTime: Beam.DateTime;

    /** Whether the schedule repeats. */
    repeats: boolean;

    /** The interval between repeated runs. */
    repeatInterval: Interval;
  }
}

function TypeFieldset(props: {reports: readonly ReportDefinition[];
    value: string; parametersId: string; onChange: (value: string) => void}):
    JSX.Element {
  return <fieldset className={css(STYLES.fieldset, STYLES.typeFieldset)}>
    <InputGroup label='Report Type'>
      <ReportTypeSelect reportTypes={props.reports} value={props.value}
        aria-controls={props.parametersId} onChange={props.onChange}/>
    </InputGroup>
  </fieldset>;
}

interface ParametersProperties {
  id: string;
  report: ReportDefinition;
  value: ReportFormTemplate.Value;
  accountModel: AccountGroupQueryModel;
  scopeModel: QueryModel<Nexus.Scope>;
  onChange: (name: string, value: ReportParameterValue) => void;
  onValidation: (name: string, error: ValidationError) => void;
}

function ParametersFieldset(props: ParametersProperties): JSX.Element {
  return <fieldset id={props.id} className={css(STYLES.fieldset)}>
    <legend className={css(STYLES.legend)}>Parameters</legend>
    <div className={css(STYLES.parameters)}>
      {props.report?.parameters.map(parameter =>
        <Parameter key={`${props.report.id}-${parameter.name}`}
          definition={parameter} value={props.value.parameters[parameter.name]}
          accountModel={props.accountModel} scopeModel={props.scopeModel}
          onChange={props.onChange} onValidation={props.onValidation}/>)}
    </div>
  </fieldset>;
}

class Parameter extends React.Component<{
    definition: ReportParameterDefinition; value: ReportParameterValue;
    accountModel: AccountGroupQueryModel; scopeModel: QueryModel<Nexus.Scope>;
    onChange: ParametersProperties['onChange'];
    onValidation: ParametersProperties['onValidation']}> {
  public render(): JSX.Element {
    return <ReportParameterInput {...this.props} onChange={this.onChange}
      onValidationChange={this.onValidation}/>;
  }

  private onChange = (value: ReportParameterValue) => {
    this.props.onChange(this.props.definition.name, value);
  };

  private onValidation = (error: ValidationError) => {
    this.props.onValidation(this.props.definition.name, error);
  };
}

function ShareFieldset(props: {value: readonly Beam.DirectoryEntry[];
    accountModel: AccountGroupQueryModel; scopeModel: QueryModel<Nexus.Scope>;
    onChange: (value: ReportParameterValue) => void;
    onValidation: (error: ValidationError) => void}): JSX.Element {
  return <fieldset className={css(STYLES.fieldset)}>
    <ReportParameterInput {...props} definition={SHARING_PARAMETER}
      onValidationChange={props.onValidation}/>
  </fieldset>;
}

class RunFieldset extends React.Component<{name: string; scheduled: boolean;
    scheduleId: string; onChange: (value: boolean) => void}> {
  public render(): JSX.Element {
    return <fieldset className={css(STYLES.fieldset, STYLES.run)}>
      <legend className={css(STYLES.legend, STYLES.runLegend)}>Run</legend>
      <div className={css(STYLES.runOptions)}>
        <RadioButton name={this.props.name} label='Now' value='now'
          checked={!this.props.scheduled} onChange={this.onNow}/>
        <RadioButton name={this.props.name} label='On a Schedule' value='later'
          checked={this.props.scheduled} onChange={this.onLater}
          aria-controls={this.props.scheduleId}/>
      </div>
    </fieldset>;
  }

  private onNow = () => {
    this.props.onChange(false);
  };

  private onLater = () => {
    this.props.onChange(true);
  };
}

function ScheduleFieldset(props: {id: string; value: ReportFormTemplate.Value;
    accountModel: AccountGroupQueryModel; scopeModel: QueryModel<Nexus.Scope>;
    error: ValidationError; onDateChange: (value: ReportParameterValue) => void;
    onValidation: (error: ValidationError) => void;
    onRepeatsChange: (value: boolean) => void;
    onIntervalChange: (value: Interval) => void;
    intervalError: ValidationError;
    onIntervalInput: React.FormEventHandler<HTMLDivElement>}): JSX.Element {
  const interval = props.value.repeatInterval;
  const error = (() => {
    if(props.intervalError) {
      return props.intervalError;
    }
    if(interval?.count == null) {
      return ValidationError.REQUIRED;
    } else if(!Number.isInteger(interval.count) || interval.count < 1) {
      return ValidationError.FORMAT;
    }
    return ValidationError.NONE;
  })();
  return <fieldset id={props.id} className={css(STYLES.fieldset,
      STYLES.schedule)}>
    <legend className={css(STYLES.legend)}>Schedule</legend>
    <ReportParameterInput definition={SCHEDULE_PARAMETER}
      value={props.value.scheduleDateTime} error={props.error}
      accountModel={props.accountModel} scopeModel={props.scopeModel}
      onChange={props.onDateChange} onValidationChange={props.onValidation}/>
    <Repeat value={props.value.repeats} onChange={props.onRepeatsChange}
      controls={`${props.id}-interval`}/>
    <Collapse open={props.value.repeats} spacing={false}>
      <div id={`${props.id}-interval`} style={{paddingTop: '10px'}}>
        <InputGroup label='Every'
            validation={{valid: error === ValidationError.NONE, error}}
            errorMessage={<InputErrorMessage error={error} label='Interval'
              value='interval'/>}>
          <IntervalInput value={interval} required
            onInput={props.onIntervalInput} onChange={props.onIntervalChange}/>
        </InputGroup>
      </div>
    </Collapse>
  </fieldset>;
}

function Repeat(props: {value: boolean; controls: string;
    onChange: (value: boolean) => void}): JSX.Element {
  return <label className={css(STYLES.repeat)}>
    <span className={css(STYLES.repeatLabel)}>Repeat</span>
    <Checkbox checked={props.value} onClick={props.onChange}
      aria-controls={props.controls}/>
  </label>;
}

function SubmitSection(props: {mode: ReportFormTemplate.Mode;
    scheduled: boolean; valid: boolean}): JSX.Element {
  const label = (() => {
    if(props.mode === ReportFormTemplate.Mode.EDIT) {
      return 'Save Changes';
    } else if(props.scheduled) {
      return 'Schedule';
    }
    return 'Generate';
  })();
  return <section className={css(STYLES.submit)}>
    <Button type='submit' label={label} disabled={!props.valid}
      style={{width: '100%'}}/>
  </section>;
}

interface CollapseState {
  open: boolean;
  height: number;
  animating: boolean;
}

class Collapse extends React.Component<{open: boolean; spacing?: boolean},
    CollapseState> {
  public static getDerivedStateFromProps(props: {open: boolean},
      state: CollapseState): Partial<CollapseState> {
    if(props.open !== state.open) {
      return {open: props.open, animating: true};
    }
    return null;
  }

  constructor(props: {open: boolean; spacing?: boolean}) {
    super(props);
    this.state = {open: props.open, height: 0, animating: false};
    this.content = React.createRef();
    this.observer = null;
    this.frame = null;
  }

  public render(): JSX.Element {
    const height = (() => {
      if(this.props.open) {
        return this.state.height;
      }
      return 0;
    })();
    const attributes: any = {};
    if(!this.props.open) {
      attributes.inert = '';
    }
    const transition = (() => {
      if(!this.state.animating) {
        return 'none';
      } else if(this.props.open) {
        return 'max-height 200ms ease-in-out';
      }
      return 'max-height 200ms ease-in-out, visibility 0s linear 200ms';
    })();
    return <div {...attributes} aria-hidden={!this.props.open}
        onTransitionEnd={this.onTransitionEnd}
        style={{maxHeight: height, overflow: 'clip',
          visibility: this.props.open && 'visible' || 'hidden',
          transition}}>
      <div ref={this.content} style={{display: 'flow-root',
          paddingBottom: (() => {
            if(this.props.spacing === false) {
              return 0;
            }
            return '30px';
          })()}}>
        {this.props.children}
      </div>
    </div>;
  }

  public componentDidMount(): void {
    this.observer = new ResizeObserver(this.scheduleMeasure);
    this.observer.observe(this.content.current);
    this.scheduleMeasure();
  }

  public componentDidUpdate(): void {
    this.scheduleMeasure();
  }

  public componentWillUnmount(): void {
    this.observer.disconnect();
    window.cancelAnimationFrame(this.frame);
  }

  private scheduleMeasure = () => {
    if(this.frame !== null) {
      return;
    }
    this.frame = window.requestAnimationFrame(() => {
      this.frame = null;
      const height = this.content.current.getBoundingClientRect().height;
      if(height !== this.state.height) {
        this.setState({height});
      }
    });
  };

  private onTransitionEnd = (event: React.TransitionEvent<HTMLDivElement>) => {
    if(event.target === event.currentTarget &&
        event.propertyName === 'max-height' && this.state.animating) {
      this.setState({animating: false});
    }
  };

  private content: React.RefObject<HTMLDivElement>;
  private observer: ResizeObserver;
  private frame: number;
}

const SHARING_PARAMETER = new ReportParameterDefinition(
  '$sharing', 'Sharing', 'DirectoryEntryList', false);
const SCHEDULE_PARAMETER = new ReportParameterDefinition(
  '$schedule', 'Date & Time', 'DateTime', true);
const STYLES = StyleSheet.create({
  container: {containerType: 'inline-size', width: '100%'},
  form: {backgroundColor: '#FFFFFF', color: '#333333',
    fontFamily: 'Roboto, system-ui, sans-serif', fontSize: '0.875rem',
    fontWeight: 400},
  heading: {fontSize: '1.125rem', fontWeight: 500, margin: 0},
  fieldset: {boxSizing: 'border-box', minWidth: 0, margin: '0 0 30px',
    padding: '30px 0 0', border: 0, borderTop: '1px solid #E6E6E6'},
  typeFieldset: {borderTop: 'none', paddingTop: 0,
    maxWidth: '100%'},
  legend: {float: 'left', width: '100%', boxSizing: 'border-box',
    padding: 0, marginBottom: '20px', color: '#4B23A0', fontWeight: 500},
  parameters: {clear: 'both', display: 'flex', flexDirection: 'column',
    gap: '10px'},
  run: {marginBottom: 0},
  runLegend: {
    '@container (min-width: 384px)': {width: '130px',
      marginBottom: 0, marginRight: '8px', lineHeight: '20px'}
  },
  runOptions: {display: 'flex', gap: '18px',
    '@container (width < 384px)': {flexDirection: 'column',
      paddingLeft: '10px', clear: 'both'},
    '@container (min-width: 384px)': {marginLeft: '138px'}},
  schedule: {marginBottom: 0},
  repeat: {display: 'grid', alignItems: 'center', width: 'fit-content',
    marginTop: '10px',
    '@container (width < 384px)': {
      gridTemplateColumns: 'minmax(0, 1fr)', rowGap: '12px'},
    '@container (min-width: 384px)': {
      gridTemplateColumns: '130px 20px', columnGap: '8px', minHeight: '34px'}},
  repeatLabel: {display: 'flex', alignItems: 'center', alignSelf: 'stretch',
    '@container (width < 384px)': {paddingInlineStart: '10px'},
    '@container (min-width: 384px)': {paddingInlineStart: 0}},
  submit: {backgroundColor: '#FFFFFF', borderTop: '1px solid #E6E6E6',
    paddingTop: '30px'}
});
