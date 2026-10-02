import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { AccountGroupListInput, AccountGroupQueryModel, ComboBox, DateInput,
  DecimalInput, InputErrorMessage, InputGroup, IntegerInput, MoneyInput,
  ScopeInput, Select, TimeOfDayInput } from '../../components';
import { DateRange, DateRangeValidation, QueryModel, ValidationError } from
  '../../models';
import { ParametersDateRangeInput } from './parameters_date_range_input';
import { ReportParameterDefinition } from './report_parameter_definition';
import { ReportParameterValue, validateReportParameter } from
  './report_parameter_value';

interface Properties {

  /** The parameter to edit. */
  definition: ReportParameterDefinition;

  /** The committed parameter value. */
  value: ReportParameterValue;

  /** The model used for account and group lookups. */
  accountModel: AccountGroupQueryModel;

  /** The model used for scope lookups. */
  scopeModel: QueryModel<Nexus.Scope>;

  /** Additional validation supplied by the form. */
  error?: ValidationError;

  /** Reports a committed value. */
  onChange: (value: ReportParameterValue) => void;

  /** Reports validity of the current draft. */
  onValidationChange: (error: ValidationError) => void;
}

interface State {
  draftError: ValidationError;
}

/** Selects a report parameter's editor using its shared type name. */
export class ReportParameterInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {draftError: ValidationError.NONE};
    this.element = React.createRef();
    this.mounted = false;
    this.emptyRange = new DateRange(null, null);
  }

  public render(): JSX.Element {
    const {definition, value} = this.props;
    if(definition.type === 'DateRange') {
      return <ParametersDateRangeInput
        value={value as DateRange ?? this.emptyRange} onChange={this.onChange}
        label={definition.label} boundsRequired={definition.required}
        style={{maxWidth: '384px'}}
        onValidationChange={this.onRangeValidation}/>;
    }
    const error = this.state.draftError || this.props.error ||
      validateReportParameter(definition, value);
    return <div ref={this.element} onInput={this.onInput} onBlur={this.onBlur}>
      <InputGroup label={definition.label}
          validation={{valid: error === ValidationError.NONE, error}}
          errorMessage={<InputErrorMessage label={definition.label}
            value={getValueLabel(definition.type)} error={error}/>}>
        {this.renderInput()}
      </InputGroup>
    </div>;
  }

  public componentDidMount(): void {
    this.mounted = true;
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.value !== this.props.value) {
      this.setError(ValidationError.NONE);
      if(this.element.current?.contains(document.activeElement)) {
        this.onInput();
      }
    }
  }

  public componentWillUnmount(): void {
    this.mounted = false;
  }

  private renderInput(): React.ReactElement {
    const {definition, value, accountModel, scopeModel} = this.props;
    switch(definition.type) {
      case 'DirectoryEntry':
        return <ComboBox value={value as Beam.DirectoryEntry}
          model={accountModel} getLabel={getEntryLabel} isEqual={equalEntries}
          onChange={this.onChange}
          invalidMessage='A valid account or group is required'
          onValidationError={this.onLookupError}
          onResolvingChange={this.onResolving}/>;
      case 'DirectoryEntryList':
        return <AccountGroupListInput
          value={value as Beam.DirectoryEntry[] ?? []}
          model={accountModel} onChange={this.onChange}/>;
      case 'Date':
        return <DateInput value={value as Beam.Date} onChange={this.onChange}/>;
      case 'DateTime':
        return <DateTimeField value={value as Beam.DateTime}
          onChange={this.onChange}/>;
      case 'Time':
        return <TimeOfDayInput value={value as Beam.Duration}
          onChange={this.onChange}/>;
      case 'Scope':
        return <ScopeInput value={value as Nexus.Scope} model={scopeModel}
          onChange={this.onChange}/>;
      case 'Decimal':
        return <DecimalInput value={value as number} onChange={this.onChange}/>;
      case 'Integer':
        return <IntegerInput value={value as number} onChange={this.onChange}/>;
      case 'Money':
        return <MoneyInput value={value as Nexus.Money}
          onChange={this.onChange}/>;
      case 'Currency':
        return <Select value={(value as Nexus.Currency)?.code.toString() ?? ''}
            onChange={this.onCurrencyChange}>
          <option value=''/>
          {[...CURRENCIES].map(entry =>
            <option key={entry.currency.code} value={entry.currency.code}>
              {entry.code}
            </option>)}
        </Select>;
      default:
        throw new Error(
          `Unsupported report parameter type: ${definition.type}`);
    }
  }

  private setError(error: ValidationError): void {
    if(error !== this.state.draftError) {
      this.setState({draftError: error});
    }
    this.props.onValidationChange(error);
  }

  private readDraft(commit: boolean): void {
    const type = this.props.definition.type;
    if(type === 'DirectoryEntry') {
      const input = this.element.current.querySelector('input');
      if(input.value === '') {
        this.setError(validateReportParameter(this.props.definition, null));
        if(commit) {
          this.props.onChange(null);
        }
      } else if(input.value !==
          (this.props.value as Beam.DirectoryEntry)?.name) {
        this.setError(ValidationError.FORMAT);
      } else {
        this.setError(ValidationError.NONE);
      }
      return;
    }
    if(!['Date', 'Time', 'DateTime', 'Integer', 'Decimal', 'Money'].
        includes(type)) {
      return;
    }
    const inputs = Array.from(this.element.current.querySelectorAll('input'));
    const parts = inputs.map(input => input.value);
    if(parts.every(part => part === '')) {
      this.setError(validateReportParameter(this.props.definition, null));
      if(commit) {
        this.props.onChange(null);
      }
      return;
    }
    if(parts.some(part =>
        part.trim() === '' || !Number.isFinite(Number(part)))) {
      this.setError(ValidationError.FORMAT);
      return;
    }
    const values = parts.map(Number);
    let value: ReportParameterValue;
    if(type === 'Integer' || type === 'Decimal') {
      value = values[0];
    } else if(type === 'Money') {
      value = Nexus.Money.parse(parts[0]);
    } else {
      let time: Beam.Duration = null;
      if(type !== 'Date') {
        const start = values.length - 3;
        const [hours, minutes, seconds] = values.slice(start);
        if(hours < 1 || hours > 12 || minutes < 0 || minutes > 59 ||
            seconds < 0 || seconds > 59) {
          this.setError(ValidationError.FORMAT);
          return;
        }
        const period =
          Number(this.element.current.querySelector('select').value);
        time = Beam.Duration.HOUR.multiply(hours % 12 + period * 12).
          add(Beam.Duration.MINUTE.multiply(minutes)).
          add(Beam.Duration.SECOND.multiply(seconds));
      }
      if(type === 'Time') {
        value = time;
      } else {
        const date = new Beam.Date(values[0], values[1], values[2]);
        if(type === 'Date') {
          value = date;
        } else {
          value = new Beam.DateTime(date, time);
        }
      }
    }
    const error = validateReportParameter(this.props.definition, value);
    this.setError(error);
    if(commit && error === ValidationError.NONE) {
      this.props.onChange(value);
    }
  }

  private onInput = () => {
    queueMicrotask(() => {
      if(this.mounted) {
        this.readDraft(false);
      }
    });
  };

  private onBlur = (event: React.FocusEvent) => {
    if(!event.currentTarget.contains(event.relatedTarget as Node)) {
      queueMicrotask(() => {
        if(this.mounted) {
          this.readDraft(true);
        }
      });
    }
  };

  private onChange = (value: ReportParameterValue) => {
    this.setError(ValidationError.NONE);
    this.props.onChange(value ?? null);
  };

  private onCurrencyChange = (value: string) => {
    if(value === '') {
      this.onChange(null);
    } else {
      this.onChange(new Nexus.Currency(Number(value)));
    }
  };

  private onRangeValidation = (validation: DateRangeValidation) => {
    let error = ValidationError.NONE;
    if(!validation.valid) {
      const range = this.props.value as DateRange;
      if(!this.props.definition.required && (!range ||
          !range.start && !range.end) && validation.error ===
          DateRangeValidation.Error.REQUIRED) {
        this.props.onValidationChange(error);
        return;
      }
      if(validation.error === DateRangeValidation.Error.REQUIRED) {
        error = ValidationError.REQUIRED;
      } else if(validation.error === DateRangeValidation.Error.OUT_OF_RANGE) {
        error = ValidationError.OUT_OF_RANGE;
      } else {
        error = ValidationError.FORMAT;
      }
    }
    this.props.onValidationChange(error);
  };

  private onLookupError = (message: string) => {
    this.setError(message && ValidationError.FORMAT || ValidationError.NONE);
  };

  private onResolving = (resolving: boolean) => {
    if(resolving) {
      this.setError(ValidationError.FORMAT);
    } else {
      this.onInput();
    }
  };

  private element: React.RefObject<HTMLDivElement>;
  private mounted: boolean;
  private emptyRange: DateRange;
}

interface DateTimeProperties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'onChange'> {
  value: Beam.DateTime;
  onChange: (value: Beam.DateTime) => void;
}

class DateTimeField extends React.Component<DateTimeProperties,
    {date: Beam.Date; time: Beam.Duration; revision: number}> {
  constructor(props: DateTimeProperties) {
    super(props);
    this.state = {date: props.value?.date, time: props.value?.timeOfDay,
      revision: 0};
    this.published = undefined;
  }

  public render(): JSX.Element {
    const {value, style, id, onChange, ...attributes} = this.props;
    return <div {...attributes} style={{display: 'flex',
        flexDirection: 'column', gap: '10px', ...style}}>
      <DateInput key={`date-${this.state.revision}`} id={id}
        value={this.state.date}
        aria-invalid={attributes['aria-invalid']}
        aria-describedby={attributes['aria-describedby']}
        onChange={this.onDateChange}/>
      <TimeOfDayInput key={`time-${this.state.revision}`}
        value={this.state.time}
        aria-invalid={attributes['aria-invalid']}
        aria-describedby={attributes['aria-describedby']}
        onChange={this.onTimeChange}/>
    </div>;
  }

  public componentDidUpdate(previous: DateTimeProperties): void {
    if(previous.value !== this.props.value &&
        this.props.value !== this.published) {
      this.setState(state => ({date: this.props.value?.date,
        time: this.props.value?.timeOfDay, revision: state.revision + 1}));
    }
    this.published = undefined;
  }

  private publish = () => {
    if(this.state.date && this.state.time) {
      this.published = new Beam.DateTime(this.state.date, this.state.time);
    } else {
      this.published = null;
    }
    this.props.onChange(this.published);
  };

  private onDateChange = (date: Beam.Date) => {
    this.setState({date}, this.publish);
  };

  private onTimeChange = (time: Beam.Duration) => {
    this.setState({time}, this.publish);
  };

  private published: Beam.DateTime;
}

function getEntryLabel(entry: Beam.DirectoryEntry): string {
  return entry.name;
}

function equalEntries(left: Beam.DirectoryEntry, right: Beam.DirectoryEntry):
    boolean {
  return left.equals(right);
}

function getValueLabel(type: string): string {
  if(type === 'DirectoryEntry' || type === 'DirectoryEntryList') {
    return 'account or group';
  } else if(type === 'Scope') {
    return 'country, venue, or ticker';
  } else if(type === 'DateTime') {
    return 'date and time';
  }
  return 'value';
}

const CURRENCIES = Nexus.buildCurrencyDatabase();
