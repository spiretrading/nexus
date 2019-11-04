import * as React from 'react';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { DisplaySize } from '../../../display_size';
import { RuleMode } from './rule_execution_drop_down';
import { RuleRow } from './rule_row';
import {RulesList} from './rules_list';
import { NewRuleButton } from '.';


interface Properties {
  
  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;
  currencyDatabase?: Nexus.CurrencyDatabase;
  complianceList: Nexus.ComplianceRuleEntry[];
}

interface State {
  complianceList: Nexus.ComplianceRuleEntry[];
}

/* Displays the compliance page.*/
export class CompliancePage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      complianceList: this.props.complianceList.slice()
    };
  }

  public render(): JSX.Element {
    const content = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return CompliancePage.STYLE.smallContent;
      } else if(this.props.displaySize === DisplaySize.MEDIUM) {
        return CompliancePage.STYLE.mediumContent;
      } else {
        return CompliancePage.STYLE.largeContent;
      }
    })();
    const rules = (() => {
      if(this.state.complianceList === []) {
        return null;
      } else {
        return (
          <RulesList 
            displaySize={this.props.displaySize}
            currencyDatabase={this.props.currencyDatabase}
            complianceList={this.state.complianceList}/>);
      }
    })();
    return(
      <div style={CompliancePage.STYLE.wrapper}>
        <div style={CompliancePage.STYLE.filler}/>
        <div style={content}>
          {rules}
          <div style={CompliancePage.STYLE.paddingMedium}/>
          <NewRuleButton displaySize={this.props.displaySize} />
          <div style={CompliancePage.STYLE.paddingLarge}/>
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
    paddingMedium: {
      width: '100%',
      height: '20px'
    },
    paddingLarge: {
      width: '100%',
      height: '30px'
    },
    smallContent: {
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      minWidth: '284px',
      maxWidth: '424px',
      width: '100%',
    },
    mediumContent: {
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      width: '732px'
    },
    largeContent: {
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      width: '1000px'
    },
    newRuleRow: {
      height: '20px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      font: '400 16px Roboto',
    },
    newRuleText: {
      font: '400 14px Roboto',
      height: '20px',
      paddingLeft: '18px'
    }
  };
}
