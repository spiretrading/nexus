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

  /** The list of compliance rules to display and edit. */
  entries: Nexus.ComplianceRuleEntry[];

  /** The list of rule schemas. Used in adding new rules. */
  schemas: Nexus.ComplianceRuleSchema[];

  /** */
  onRuleAdd?: (newSchema: Nexus.ComplianceRuleSchema) => void;

  onRuleChanged?: (index: number, newRule: Nexus.ComplianceRuleEntry) => void;
}

interface State {
  isAddRuleModalOpen: boolean;
}

/* Displays the compliance page.*/
export class CompliancePage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isAddRuleModalOpen: false
    };
    this.onRuleChange = this.onRuleChange.bind(this);
    this.onToggleAddRuleModal = this.onToggleAddRuleModal.bind(this);
    this.onAddNewRule = this.onAddNewRule.bind(this);
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
            complianceList={this.props.entries}
            onChange={this.onRuleChange}/>
          <div style={CompliancePage.STYLE.paddingMedium}/>
          <NewRuleButton displaySize={this.props.displaySize}
            isOpen={this.state.isAddRuleModalOpen}
            onToggleModal={this.onToggleAddRuleModal}
            onAddNewRule={this.onAddNewRule}
            schemas={this.props.schemas}/>
          <div style={CompliancePage.STYLE.paddingLarge}/>
        </div>
        <div style={CompliancePage.STYLE.filler}/>
      </div>);
  }

  private onRuleChange(ruleIndex: number, newRule: Nexus.ComplianceRuleEntry) {
    this.props.onRuleChanged(ruleIndex, newRule);
  }

  private onToggleAddRuleModal() {
    ///put callback here 
    this.setState({isAddRuleModalOpen: !this.state.isAddRuleModalOpen});
  }

  private onAddNewRule(schema: Nexus.ComplianceRuleSchema) {
    this.props.onRuleAdd(schema);
  }

  private static readonly STYLE = {
    wrapper: {
      paddingTop: '18px',
      paddingLeft: '18px',
      paddingRight: '18px',
      paddingBottom: '60px',
      height: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row'
    },
    filler: {
      flexGrow: 1,
      flexShrink: 1
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
      width: '100%'
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
    }
  };
}
