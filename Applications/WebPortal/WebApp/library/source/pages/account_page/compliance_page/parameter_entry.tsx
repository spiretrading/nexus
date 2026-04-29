import * as Nexus from 'nexus';
import * as React from 'react';
import { Checkbox, CurrencySelect, DateTimeInput, DecimalInput, DisplaySize,
  Input, MoneyInput, ScopeInput, TickersInput,
  TimeOfDayInput } from '../../..';

interface Properties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** The parameter to be displayed. */
  parameter?: Nexus.ComplianceParameter;

  /** The set of available currencies to select. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** Indicates if the component is readonly. */
  readonly?: boolean;

  /** The event handler called when the parameter changes. */
  onChange?: (newParameter: Nexus.ComplianceParameter) => void;
}

/** Displays a specific parameter. */
export class ParameterEntry extends React.Component<Properties> {
  public static readonly defaultProps = {
    onChange: () => {}
  };

  public render(): JSX.Element {
    const rowStyle = (() => {
      if(this.props.parameter.value.type !== 
          Nexus.ComplianceValue.Type.DATE_TIME) {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return ParameterEntry.STYLE.rowSmall;
        } else {
          return ParameterEntry.STYLE.rowLarge;
        }
      } else {
        return ParameterEntry.STYLE.dateTimeRow;
      }
    })();
    const inputWrapper = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ParameterEntry.STYLE.inputSmall;
      } else {
        return ParameterEntry.STYLE.inputLarge;
      }
    })();
    const input = (() => {
      switch(this.props.parameter.value.type) {
        case Nexus.ComplianceValue.Type.BOOLEAN:
          return <Checkbox
            onClick={this.onCheckboxChange}
            disabled={this.props.readonly}
            checked={this.props.parameter.value.value}/>;
        case Nexus.ComplianceValue.Type.CURRENCY:
          return <CurrencySelect
            value={this.props.parameter.value.value}
            style={inputWrapper}
            onChange={this.onChange}
            readOnly={this.props.readonly}
            currencyDatabase={this.props.currencyDatabase}/>;
        case Nexus.ComplianceValue.Type.DATE_TIME:
          return <DateTimeInput
            value={this.props.parameter.value.value}
            readOnly={this.props.readonly}
            onChange={this.onChange}/>;
        case Nexus.ComplianceValue.Type.DOUBLE:
          return <DecimalInput
            value={this.props.parameter.value.value}
            readOnly={this.props.readonly}
            onChange={this.onChange}/>;
        case Nexus.ComplianceValue.Type.DURATION:
          return <TimeOfDayInput
            value={this.props.parameter.value.value}
            readOnly={this.props.readonly}
            onChange={this.onChange}/>;
        case Nexus.ComplianceValue.Type.MONEY:
          return <MoneyInput
            value={this.props.parameter.value.value}
            readOnly={this.props.readonly}
            onChange={this.onChange}/>;
        case Nexus.ComplianceValue.Type.QUANTITY:
          return <DecimalInput
            value={this.props.parameter.value.value}
            readOnly={this.props.readonly}
            onChange={this.onChange}/>;
        case Nexus.ComplianceValue.Type.STRING:
          return <Input
            value={this.props.parameter.value.value}
            onChange={(e: React.ChangeEvent<HTMLInputElement>) =>
              this.onChange(e.target.value)}
            readOnly={this.props.readonly}
            style={inputWrapper}/>;
        case Nexus.ComplianceValue.Type.TICKER:
          return <TickersInput
            displaySize={this.props.displaySize}
            onChange={(values: Nexus.Ticker[]) =>
              this.onChange(values[0])}
            value={this.props.parameter.value.value ?
              [this.props.parameter.value.value] : []}/>;
        case Nexus.ComplianceValue.Type.SCOPE:
          return <ScopeInput
            displaySize={this.props.displaySize}
            readOnly={this.props.readonly}
            onChange={this.onChange}
            value={this.props.parameter.value.value}/>;
        case Nexus.ComplianceValue.Type.LIST:
          if(this.props.parameter.value.value.length > 0) {
            if(this.props.parameter.value.value[0].type ===
                Nexus.ComplianceValue.Type.TICKER) {
              return <TickersInput
                displaySize={this.props.displaySize}
                onChange={this.onTickerListChange}
                readOnly={this.props.readonly}
                value={this.convertFromParameterList(
                  this.props.parameter.value.value)}/>;
            }
          } else {
            return <TickersInput
              displaySize={this.props.displaySize}
              onChange={this.onTickerListChange}
              readOnly={this.props.readonly}
              value={[]}/>;
          }
        default:
          return <div/>;
      }
    })();
    return (
      <div style={rowStyle}>
        <div style={ParameterEntry.STYLE.label}>
          {this.props.parameter.name}
        </div>
        <div style={inputWrapper}>
          {input}
        </div>
      </div>);
  }

  private onChange = (newValue: any) => {
    this.props.onChange(new Nexus.ComplianceParameter(this.props.parameter.name,
      new Nexus.ComplianceValue(this.props.parameter.value.type, newValue)));
  }

  private onCheckboxChange = (isChecked: boolean) => {
    this.onChange(isChecked);
  }

  private convertFromParameterList(complianceValues: Nexus.ComplianceValue[]) {
    const tickers = [];
    for(const value of complianceValues) {
      const ticker = value.value as Nexus.Ticker;
      if(!ticker.equals(Nexus.Ticker.NONE)) {
        tickers.push(ticker);
      }
    }
    return tickers;
  }

  private onTickerListChange = (newValues: Nexus.Ticker[]) => {
    const newParameterList = newValues.map(newValue => {
      return new Nexus.ComplianceValue(
        Nexus.ComplianceValue.Type.TICKER, newValue);
    });
    this.props.onChange(new Nexus.ComplianceParameter(
      this.props.parameter.name, new Nexus.ComplianceValue(
        Nexus.ComplianceValue.Type.LIST, newParameterList)));
  }

  private static readonly STYLE = {
    inputSmall: {
      minWidth: '184px',
      flexBasis: '184px',
      maxWidth: '246px',
      flexGrow: 1,
      flexShrink: 1
    },
    inputLarge: {
      width: '246px',
      minWidth: '246px',
      maxWidth: '246px',
      flexBasis: '246px',
      flexGrow: 0,
      flexShrink: 0
    },
    rowSmall: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexBasis: '284px',
      flexGrow: 1,
      flexShrink: 1,
      maxWidth: '424px',
      width: '100%',
      height: '34px',
      font: '400 14px Roboto',
      alignItems: 'center' as 'center'
    },
    rowLarge: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      width: '100%',
      height: '34px',
      font: '400 14px Roboto',
      alignItems: 'center' as 'center'
    },
    dateTimeRow: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      width: '100%',
      height: '78px',
      font: '400 14px Roboto',
      alignItems: 'center' as 'center'
    },
    label: {
      boxSizing: 'border-box' as 'border-box',
      width: '100px',
      font: '400 14px Roboto',
      flexGrow: 0,
      flexShrink: 0,
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      height: '34px',
      alignSelf: 'flex-start' as 'flex-start',
      color: '#333333',
      cursor: 'default' as 'default'
    },
  };
}
