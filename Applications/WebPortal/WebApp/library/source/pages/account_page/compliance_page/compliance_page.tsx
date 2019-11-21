import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../../display_size';
import { NewRuleButton, RulesList } from '.';

interface Properties {
  
  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** The set of available currencies to select. */
  currencyDatabase?: Nexus.CurrencyDatabase;

  /** The list of compliance rules. */
  complianceList: Nexus.ComplianceRuleEntry[];

  /** A list of rule Schemas */
  ruleSchemas: Nexus.ComplianceRuleSchema[];
}

interface State {
  complianceList: Nexus.ComplianceRuleEntry[];

  isAddNewRulewModalOpen: boolean;
}

/* Displays the compliance page.*/
export class CompliancePage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      complianceList: this.props.complianceList.slice(),
      isAddNewRulewModalOpen: false
    };
    this.onRuleChange = this.onRuleChange.bind(this);
  }

  public render(): JSX.Element {
    const contentStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return CompliancePage.STYLE.smallContent;
      } else if(this.props.displaySize === DisplaySize.MEDIUM) {
        return CompliancePage.STYLE.mediumContent;
      } else {
        return CompliancePage.STYLE.largeContent;
      }
    })();
    return (
      <div style={CompliancePage.STYLE.wrapper}>
        <div style={CompliancePage.STYLE.filler}/>
        <div style={contentStyle}>
          <RulesList 
            displaySize={this.props.displaySize}
            currencyDatabase={this.props.currencyDatabase}
            complianceList={this.state.complianceList}
            onChange={this.onRuleChange}/>
          <div style={CompliancePage.STYLE.paddingMedium}/>
          <NewRuleButton displaySize={this.props.displaySize}
            isOpen={this.state.isAddNewRulewModalOpen}
            onToggleModal={this.onToggleAddNewRule.bind(this)}
            onAddNewRule={this.onAddNewRow.bind(this)}
            ruleSchemas={this.props.ruleSchemas}/>
          <div style={CompliancePage.STYLE.paddingLarge}/>
        </div>
        <div style={CompliancePage.STYLE.filler}/>
      </div>);
  }

  private onRuleChange(ruleIndex: number, newRule: Nexus.ComplianceRuleEntry) {
    this.state.complianceList[ruleIndex] = newRule;
    this.setState({complianceList: this.state.complianceList});
  }

  private onToggleAddNewRule() {
    this.setState({isAddNewRulewModalOpen: !this.state.isAddNewRulewModalOpen});
  }

  private onAddNewRow(schema: Nexus.ComplianceRuleSchema) {
    this.state.complianceList.push(
      new Nexus.ComplianceRuleEntry (
        0,
        Beam.DirectoryEntry.makeDirectory(0, 'empty'),
        Nexus.ComplianceRuleEntry.State.DISABLED,
        schema
      )
    );
    this.setState({complianceList: this.state.complianceList});
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
