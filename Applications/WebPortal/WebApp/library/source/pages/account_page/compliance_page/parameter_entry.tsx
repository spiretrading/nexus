import * as Nexus from 'nexus';
import * as React from 'react';
import { CurrencySelectionBox, DisplaySize, DurationInput, MoneyInputBox,
  NumberInput, SecurityInput, TextInputField } from '../../..';

interface Properties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** The parameter to be displayed. */
  parameter?: Nexus.ComplianceParameter;

  /** The set of available currencies to select. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The event handler called when the parameter changes. */
  onChange?: (newParameter: Nexus.ComplianceParameter) => void;
}

/** Displays a specific parameter. */
export class ParameterEntry extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {
    const rowStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ParameterEntry.STYLE.rowSmall;
      } else {
        return ParameterEntry.STYLE.rowLarge;
      }
    })();
    const labelStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ParameterEntry.STYLE.label;
      } else {
        return ParameterEntry.STYLE.label;
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
          return <div/>;
        case Nexus.ComplianceValue.Type.CURRENCY:
          return <CurrencySelectionBox
            value={this.props.parameter.value.value}
            style={inputWrapper}
            onChange={this.onChange}
            currencyDatabase={this.props.currencyDatabase}/>;
        case Nexus.ComplianceValue.Type.DATE_TIME:
          return <div/>;
        case Nexus.ComplianceValue.Type.DOUBLE:
          return <NumberInput 
            value={this.props.parameter.value.value}
            onChange={this.onChange}/>;
        case Nexus.ComplianceValue.Type.DURATION:
          return <DurationInput
            displaySize={this.props.displaySize}
            value={this.props.parameter.value.value}
            onChange={this.onChange}/>;
        case Nexus.ComplianceValue.Type.MONEY:
          return <MoneyInputBox
            value={this.props.parameter.value.value}
            onChange={this.onChange}/>;
        case Nexus.ComplianceValue.Type.QUANTITY:
          return <NumberInput 
            value={this.props.parameter.value.value}
            onChange={this.onChange}/>;
        case Nexus.ComplianceValue.Type.STRING:
          return <TextInputField
            displaySize={this.props.displaySize}
            value={this.props.parameter.value.value}
            onInput={this.onChange}
            style={inputWrapper}/>;
        case Nexus.ComplianceValue.Type.LIST:
          if(this.props.parameter.value.value.length > 0) {
            if(this.props.parameter.value.value[0].type ===
                Nexus.ComplianceValue.Type.SECURITY) {
              return <SecurityInput
              displaySize={this.props.displaySize}
              onChange={this.onChange}
              value={this.props.parameter.value.value}/>;
            }
          } else {
            return <SecurityInput
              displaySize={this.props.displaySize}
              onChange={this.onChange}
              value={[]}/>;
          }
        default:
          return <div/>;
      }
    })();
    return (
      <div style={rowStyle}>
        <div style={labelStyle}>{this.props.parameter.name}</div>
        <div style={inputWrapper}>
          {input}
        </div>
      </div>);
  }

  private onChange(newValue: any) {
    this.props.onChange(new Nexus.ComplianceParameter(this.props.parameter.name, 
      new Nexus.ComplianceValue(this.props.parameter.value.type, newValue)));
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
    label: {
      width: '100px',
      font: '400 14px Roboto',
      flexGrow: 0,
      flexShrink: 0
    }
  };
}
