import * as React from 'react';
import { DisplaySize } from '../../../display_size';
import { DropDownButton, HLine} from '../../../components';
import { RuleExecutionDropDown, RuleMode } from './rule_execution_drop_down';
import { RuleParameters } from './rule_parameters';

interface Properties {
  
  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;
}

interface State {
  ruleMode: RuleMode;
  isExpanded: boolean;
}

export class RuleRow extends React.Component<Properties, State> {
  constructor(props: Properties){
    super(props);
    this.state = {
      ruleMode: RuleMode.PASSIVE,
      isExpanded: false
    };
    this.onRuleModeChange = this.onRuleModeChange.bind(this);
    this.onRuleOpen = this.onRuleOpen.bind(this);
  }

  public render(): JSX.Element {
    const buttonSize = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return '20px';
      } else {
        return '16px';
      }
    })();
    const boxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return RuleRow.STYLE.headerSmall;
      } else if(this.props.displaySize === DisplaySize.MEDIUM) {
        return RuleRow.STYLE.headerMedium;
      } else {
        return RuleRow.STYLE.headerLarge;
      }
    })();
    const filler = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return <div style={RuleRow.STYLE.mobilePadding}/>;
      } else {
        return null;
      }
    })();
    const line = (() => {
      if(this.state.isExpanded) {
        return <HLine/>
      } else {
        return null;
      }
    })();
    const prefixPaddingStyle = RuleRow.STYLE.prefixPadding;
    return(
      <div>
        <div style={boxStyle}>
          {filler}
          <div style={{...RuleRow.STYLE.prefix}}>
            <DropDownButton
              onClick={this.onRuleOpen}
              size={buttonSize}
              isExpanded={this.state.isExpanded}/>
            <div style={prefixPaddingStyle}/>
            <div>{'Some Rule'}</div>
          </div>
          {filler}
          <div style={RuleRow.STYLE.paddingLeft}>
            <RuleExecutionDropDown
              onChange={this.onRuleModeChange}
              value={this.state.ruleMode}
              displaySize={this.props.displaySize}/>
          </div>
          {filler}
        </div>
        <div>
          {line}
          <RuleParameters displaySize={this.props.displaySize}/>
        </div>
      </div>);
  }

  private onRuleModeChange(newMode: RuleMode) {
    this.setState({ruleMode: newMode});
  }

  private onRuleOpen(event?: React.MouseEvent<any>) {
    this.setState({isExpanded: !this.state.isExpanded});
  }

  private static readonly STYLE = {
    headerSmall: {
      minWidth: '284px',
      maxWidth: '424px',
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      height: '84px',
      font: '400 14px Roboto',
    },
    headerMedium: {
      width: '732px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      height: '54px',
      justifyContent: 'space-between' as 'space-between',
      alignItems: 'center' as 'center',
      font: '400 14px Roboto',
    },
    headerLarge: {
      width: '1000px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      height: '54px',
      justifyContent: 'space-between' as 'space-between',
      alignItems: 'center' as 'center',
      font: '400 14px Roboto',
    },
    prefix: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row'
    },
    paddingLeft: {
      paddingLeft: '38px'
    },
    prefixPadding: {
      height: '20px',
      width: '18px'
    },
    mobilePadding: {
      height: '10px'
    }
  };
}