import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../..';
import { ParameterEntry } from './parameter_entry';

interface Properties {
  
  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** The schema that provides the paramters. */
  schema: Nexus.ComplianceRuleSchema;

  /** The set of available currencies to select. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The event handler called when the schema changes. */
  onChange?: (schema: Nexus.ComplianceRuleSchema) => void;
}

/** Displays the a list of parameters associated with a schema. */
export class ParametersList extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {
    const headerStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ParametersList.STYLE.headerSmall;
      } else {
        return ParametersList.STYLE.headerLarge;
      }
    })();
    const leftPadding = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return null;
      } else {
        return ParametersList.STYLE.largeWrapper;
      }
    })();
    const parameterEntries = [] as JSX.Element[];
    for(let i = 0; i < this.props.schema.parameters.length; ++i) {
      const rule = this.props.schema.parameters[i];
      if(this.props.schema.parameters.indexOf(rule) !== 0) {
        parameterEntries.push(
          <div style={ParametersList.STYLE.fillerBetweenRows}/>);
      }
      parameterEntries.push(<ParameterEntry 
        currencyDatabase={this.props.currencyDatabase}
        displaySize={this.props.displaySize}
        onChange={this.onChange.bind(i)}
        parameter={rule}/>);
    }
    return (
      <div style={leftPadding}>
        <div id={'topFiller'}/>
        <div style={headerStyle}>Parameters</div>
        <div id={'moreFiller'}/>
        {parameterEntries}
        <div style={ParametersList.STYLE.bottomFiller}/>
      </div>);
  }

  private onChange(parameterIndex: number, 
      parameter: Nexus.ComplianceParameter) {
    const newParameters = [] as Nexus.ComplianceParameter[];
    for(let i = 0; i < this.props.schema.parameters.length; ++i) {
      if(parameterIndex === i) {
        newParameters[i] = parameter;
      } else {
        newParameters[i] = this.props.schema.parameters[i];
      }
    }
    const newSchema = new Nexus.ComplianceRuleSchema(
      this.props.schema.name, newParameters);
    this.props.onChange(newSchema);
  }

  private static readonly STYLE = {
    largeWrapper: {
      paddingLeft: '38px'
    },
    rowSmall: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      minWidth: '284px',
      maxWidth: '424px',
      width: '100%',
      height: '34px',
      font: '400 16px Roboto',
      alignItems: 'center' as 'center'
    },
    rowLarge: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      width: '246px',
      minWidth: '246px',
      maxWidtH: '246px',
      height: '34px',
      font: '400 14px Roboto',
      alignItems: 'center' as 'center'
    },
    fillerBetweenRows : {
      height: '10px',
      width: '100%'
    },
    bottomFiller: {
      height: '30px'
    },
    label: {
      width: '100px',
      font: '400 14px Roboto',
    },
    headerSmall: {
      color: '#4B23A0',
      font: '500 14px Roboto',
      marginTop: '10px',
      marginBottom: '18px'
    },
    headerLarge: {
      color: '#4B23A0',
      font: '500 14px Roboto',
      marginTop: '10px',
      marginBottom: '18px'
    }
  };
}
