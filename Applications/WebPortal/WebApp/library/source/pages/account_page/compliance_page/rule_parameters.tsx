import * as React from 'react';
import * as Nexus from 'nexus';
import { DisplaySize } from '../../../display_size';
import { DropDownButton, CurrencySelectionBox, MoneyInputBox, TextInputField}
  from '../../../components';

interface Properties {
  
  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;
}

export class RuleParameters extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    const rowStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return RuleParameters.STYLE.rowSmall;
      } else {
        return null;
      }
    })();
    const labelStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return RuleParameters.STYLE.label;
      } else {
        return null;
      }
    })();
    return (
      <div>
        <div id={'topFiller'}/>
        <div is={'Add some margins'}>Parameters</div>
        <div id={'moreFiller'}/>
        <div style={rowStyle}>
          <div style={labelStyle}>Currency</div>
          <div style={{flexGrow: 1}}>
          <CurrencySelectionBox
            currencyDatabase={Nexus.buildDefaultCurrencyDatabase()}/>
          </div>
        </div>
        <div style={RuleParameters.STYLE.fillerBetweenRows}/>
        <div style={rowStyle}>
          <div style={labelStyle}>Buying Power</div>
          <div style={{flexGrow: 1}}><MoneyInputBox/>
          </div>
        </div>
        <div style={RuleParameters.STYLE.fillerBetweenRows}/>
        <div style={rowStyle}>
          <div style={labelStyle}>Symbols</div>
          <div style={{flexGrow: 1}}>
          <TextInputField
            displaySize={this.props.displaySize}/>
            </div>
        </div>
        <div id={'bottomFiller'}/>
      </div>);
  }

    private static readonly STYLE = {
      rowSmall: {
        display: 'flex' as 'flex',
        flexDirection: 'row' as 'row',
        minWidth: '284px',
        maxWidth: '424px',
        width: '100%',
        height: '34px',
        //justifyContent: 'space-between' as 'space-between',
        alignItems: 'center' as 'center'
      },
      fillerBetweenRows : {
        height: '10px',
        width: '100%'
      },
      label: {
        width: '100px'
      }
    };
}