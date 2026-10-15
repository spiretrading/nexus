import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { AccountGroupListInput, CurrencySelect, DateInput, DateTimeInput,
  DecimalInput, Input, InputErrorMessage, InputGroup, IntegerInput, Interval,
  IntervalInput, LocalAccountGroupQueryModel, LocalTickerQueryModel, MoneyInput,
  ScopeInput, ScopeQueryModel, TimeOfDayInput, ValidationError } from
    'web_portal';

interface Properties {

  /** The kind of input to demonstrate. */
  inputType: InputGroupExample.InputType;

  /** The label to display. */
  label: string;

  /** An existing child ID, or empty to use the group's generated ID. */
  inputId: string;

  /** The validation error to demonstrate. */
  error: ValidationError;

  /** How error visibility is controlled. */
  errorDisplay: InputGroupExample.ErrorDisplay;

  /** Custom error text, or empty to use InputErrorMessage. */
  customError: string;

  /** Whether the input is read-only. */
  readOnly: boolean;

  /** Whether the input is disabled. */
  disabled: boolean;

  /** Called when the input value changes. */
  onChange?: (value: unknown) => void;

  /** Called when the group requests validation. */
  onValidate?: () => void;
}

interface State {
  values: {[type: number]: any};
}

/** Demonstrates InputGroup with independently typed input components. */
export class InputGroupExample extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {values: {
      [InputGroupExample.InputType.TEXT]: '',
      [InputGroupExample.InputType.INTEGER]: 10,
      [InputGroupExample.InputType.DECIMAL]: 1.5,
      [InputGroupExample.InputType.MONEY]: Nexus.Money.parse('100.00'),
      [InputGroupExample.InputType.DATE]: Beam.Date.today(),
      [InputGroupExample.InputType.DATE_TIME]:
        new Beam.DateTime(Beam.Date.today(), Beam.Duration.HOUR.multiply(9)),
      [InputGroupExample.InputType.TIME]: Beam.Duration.HOUR.multiply(9),
      [InputGroupExample.InputType.INTERVAL]:
        new Interval(1, Interval.Unit.DAY),
      [InputGroupExample.InputType.CURRENCY]: new Nexus.Currency(840),
      [InputGroupExample.InputType.ACCOUNT_GROUP_LIST]: [],
      [InputGroupExample.InputType.SCOPE]: new Nexus.Scope()
    }};
    this.currencyDatabase = Nexus.buildCurrencyDatabase();
    this.accountModel = new LocalAccountGroupQueryModel([
      Beam.DirectoryEntry.makeAccount(1, 'Alice'),
      Beam.DirectoryEntry.makeDirectory(2, 'Alpha Group')]);
    this.scopeModel = new ScopeQueryModel(new LocalTickerQueryModel(
      ['ABX.TSX', 'BMO.TSX'].map(ticker => Nexus.Ticker.parse(ticker))));
  }

  public render(): JSX.Element {
    const showError = (() => {
      if(this.props.errorDisplay === InputGroupExample.ErrorDisplay.SHOW) {
        return true;
      } else if(this.props.errorDisplay ===
          InputGroupExample.ErrorDisplay.HIDE) {
        return false;
      }
      return undefined;
    })();
    const errorMessage = (() => {
      if(this.props.customError) {
        return <span style={{color: '#E63F44'}}>{this.props.customError}</span>;
      }
      return <InputErrorMessage label={this.props.label}
        error={this.props.error}
        value='value' start='Start' end='End'/>;
    })();
    return <InputGroup label={this.props.label}
        validation={{valid: this.props.error === ValidationError.NONE,
          error: this.props.error, showError}}
        errorMessage={errorMessage} onValidate={this.props.onValidate}>
      {this.renderInput()}
    </InputGroup>;
  }

  private renderInput(): React.ReactElement {
    const properties = {
      id: this.props.inputId || undefined,
      value: this.state.values[this.props.inputType],
      readOnly: this.props.readOnly,
      disabled: this.props.disabled,
      onChange: this.onChange
    };
    switch(this.props.inputType) {
      case InputGroupExample.InputType.INTEGER:
        return <IntegerInput {...properties}/>;
      case InputGroupExample.InputType.DECIMAL:
        return <DecimalInput {...properties}/>;
      case InputGroupExample.InputType.MONEY:
        return <MoneyInput {...properties}/>;
      case InputGroupExample.InputType.DATE:
        return <DateInput {...properties}/>;
      case InputGroupExample.InputType.DATE_TIME:
        return <DateTimeInput {...properties}/>;
      case InputGroupExample.InputType.TIME:
        return <TimeOfDayInput {...properties}/>;
      case InputGroupExample.InputType.INTERVAL:
        return <IntervalInput {...properties}/>;
      case InputGroupExample.InputType.CURRENCY:
        return <CurrencySelect {...properties}
          currencyDatabase={this.currencyDatabase}/>;
      case InputGroupExample.InputType.ACCOUNT_GROUP_LIST:
        return <AccountGroupListInput {...properties}
          model={this.accountModel}/>;
      case InputGroupExample.InputType.SCOPE:
        return <ScopeInput {...properties} model={this.scopeModel}/>;
      default:
        return <Input {...properties} onChange={this.onTextChange}/>;
    }
  }

  private onChange = (value: unknown) => {
    this.setState(state => ({values: {
      ...state.values, [this.props.inputType]: value
    }}));
    this.props.onChange?.(value);
  };

  private onTextChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.onChange(event.target.value);
  };

  private currencyDatabase: Nexus.CurrencyDatabase;
  private accountModel: LocalAccountGroupQueryModel;
  private scopeModel: ScopeQueryModel;
}

export namespace InputGroupExample {

  /** The available demo input components. */
  export enum InputType {
    TEXT,
    INTEGER,
    DECIMAL,
    MONEY,
    DATE,
    DATE_TIME,
    TIME,
    INTERVAL,
    CURRENCY,
    ACCOUNT_GROUP_LIST,
    SCOPE
  }

  /** The available error presentation modes. */
  export enum ErrorDisplay {
    AUTOMATIC,
    SHOW,
    HIDE
  }
}
