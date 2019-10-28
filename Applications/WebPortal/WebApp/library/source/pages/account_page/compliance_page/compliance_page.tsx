import * as React from 'react';
import { DisplaySize } from '../../../display_size';
import { RuleMode } from './rule_execution_drop_down';
import { RuleRow } from './rule_row';


interface Properties {
  
  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;
}

/* Displays the compliance page.*/
export class CompliancePage extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      ruleMode: RuleMode.PASSIVE
    };
    
  }

  public render(): JSX.Element {
    const newRuleTextStyle = CompliancePage.STYLE.newRuleTextLarge;
    return(
      <div style={CompliancePage.STYLE.wrapper}>
        <div style={CompliancePage.STYLE.filler}/>
        <div style={CompliancePage.STYLE.content}>
          <RuleRow displaySize={this.props.displaySize}/>
        </div>
        <div style={CompliancePage.STYLE.filler}/>
      </div>);
  }

  private static readonly STYLE = {
    wrapper: {
      paddingTop: '18px',
      paddingLeft: '18px',
      paddingRight: '18px',
      paddingBottom: '60px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row'
    },
    filler: {
      flexGrow: 1,
      flexShrink: 1,
    },
    content: {
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      width: '732px'
    },
    newRuleRow: {
      height: '20px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      font: '400 16px Roboto',
    },
    newRuleTextLarge: {
      font: '400 14px Roboto',
      height: '20px',
      paddingLeft: '18px'
    },
  }
}