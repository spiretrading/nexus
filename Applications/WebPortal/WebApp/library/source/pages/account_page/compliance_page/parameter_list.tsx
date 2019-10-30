import * as React from 'react';
import * as Nexus from 'nexus';
import { DisplaySize } from '../../../display_size';
import { DropDownButton, CurrencySelectionBox, MoneyInputBox, TextInputField}
  from '../../../components';
import {ParameterEntry} from './parameter_entry';
import { ComplianceRuleEntry, ComplianceRuleSchema } from 'nexus';

interface Properties {
  
  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;
  schema: ComplianceRuleSchema;
}

export class ParametersList extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    const rowStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ParametersList.STYLE.rowSmall;
      } else {
        return ParametersList.STYLE.rowLarge;
      }
    })();
    const headerStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ParametersList.STYLE.headerSmall;
      } else {
        return ParametersList.STYLE.headerLarge;
      }
    })();
    const labelStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ParametersList.STYLE.label;
      } else {
        return ParametersList.STYLE.label;
      }
    })();
    const leftPadding = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return null;
      } else {
        return ParametersList.STYLE.largeWrapper;
      }
    })();
    const thing = [] as any[];
    for(const rule of this.props.schema.parameters) {
      thing.push(<ParameterEntry displaySize={this.props.displaySize}/>);
    };
    return (
      <div style={leftPadding}>
        <div id={'topFiller'}/>
        <div style={headerStyle}>Parameters</div>
        <div id={'moreFiller'}/>
        <div style={rowStyle}>
          <div style={labelStyle}>Currency</div>
          <div style={{flexGrow: 1}}>
          <CurrencySelectionBox
            currencyDatabase={Nexus.buildDefaultCurrencyDatabase()}/>
          </div>
        </div>
        <div style={ParametersList.STYLE.fillerBetweenRows}/>
        <div style={rowStyle}>
          <div style={labelStyle}>Buying Power</div>
          <div style={{flexGrow: 1}}><MoneyInputBox/>
          </div>
        </div>
        <div style={ParametersList.STYLE.fillerBetweenRows}/>
        <div style={rowStyle}>
          <div style={labelStyle}>Symbols</div>
          <div style={{flexGrow: 1}}>
          <TextInputField
            displaySize={this.props.displaySize}/>
            </div>
        </div>
        <div style={ParametersList.STYLE.bottomFiller}/>
      </div>);
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
        width: '100%',
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
