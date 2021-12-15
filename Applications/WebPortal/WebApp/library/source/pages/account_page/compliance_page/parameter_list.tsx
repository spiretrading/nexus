import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../..';
import { ParameterEntry } from './parameter_entry';

interface Properties {
  
  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** The schema that provides the parameters. */
  schema: Nexus.ComplianceRuleSchema;

  /** The set of available currencies to select. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** Indicates if the component is readonly. */
  readonly?: boolean;

  /** The event handler called when the schema changes. */
  onChange?: (schema: Nexus.ComplianceRuleSchema) => void;
}

/** Displays the a list of parameters associated with a schema. */
export class ParametersList extends React.Component<Properties> {
  public static readonly defaultProps = {
    onChange: () => {}
  };

  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    const leftPadding = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return null;
      } else {
        return ParametersList.STYLE.largeWrapper;
      }
    })();
    const parameterEntries = [] as JSX.Element[];
    for(let i = 0; i < this.props.schema.parameters.length; ++i) {
      const parameter = this.props.schema.parameters[i];
      if(this.props.schema.parameters.indexOf(parameter) !== 0) {
        parameterEntries.push(
          <div key={parameterEntries.length}
            style={ParametersList.STYLE.fillerBetweenRows}/>);
      }
      parameterEntries.push(<ParameterEntry
        key={parameterEntries.length}
        currencyDatabase={this.props.currencyDatabase}
        displaySize={this.props.displaySize}
        readonly={this.props.readonly}
        onChange={(parameter) => this.onChange(i, parameter)}
        parameter={parameter}/>);
    }
    return (
      <div style={leftPadding}>
        <div/>
        <div style={ParametersList.STYLE.header}>Parameters</div>
        <div/>
        {parameterEntries}
        <div style={ParametersList.STYLE.bottomFiller}/>
      </div>);
  }

  private onChange =
      (parameterIndex: number, parameter: Nexus.ComplianceParameter) => {
    const newParameters = [] as Nexus.ComplianceParameter[];
    for(let i = 0; i < this.props.schema.parameters.length; ++i) {
      if(parameterIndex === i) {
        newParameters[i] = parameter;
      } else {
        newParameters[i] = this.props.schema.parameters[i];
      }
    }
    const newSchema =
      new Nexus.ComplianceRuleSchema(this.props.schema.name, newParameters);
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
      maxWidth: '246px',
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
    header: {
      color: '#4B23A0',
      font: '500 14px Roboto',
      marginTop: '10px',
      marginBottom: '18px',
      cursor: 'default' as 'default'
    }
  };
}
