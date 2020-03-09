import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../../display_size';
import { NewRuleModal, RulesList } from '.';

interface Properties {
  
  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** The set of available currencies to select. */
  currencyDatabase?: Nexus.CurrencyDatabase;

  /** The list of compliance rules to display and edit. */
  entries: Nexus.ComplianceRuleEntry[];

  /** The list of rule schemas. Used in adding new rules. */
  schemas: Nexus.ComplianceRuleSchema[];

  /** Determines if the component is readonly. */
  readonly?: boolean;

  /** The callback for adding the rule.*/
  onRuleAdd?: (newSchema: Nexus.ComplianceRuleSchema) => void;

  /** The callback for updating a changed rule. */
  onRuleChange?: (updatedRule: Nexus.ComplianceRuleEntry) => void;
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
    this.onToggleAddRuleModal = this.onToggleAddRuleModal.bind(this);
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
      <div style={contentStyle}>
        <RulesList
          displaySize={this.props.displaySize}
          currencyDatabase={this.props.currencyDatabase}
          complianceList={this.props.entries}
          onChange={this.props.onRuleChange}/>
        <div style={CompliancePage.STYLE.paddingMedium}/>
        <NewRuleModal displaySize={this.props.displaySize}
          isOpen={this.state.isAddRuleModalOpen}
          onToggleModal={this.onToggleAddRuleModal}
          onAddNewRule={this.props.onRuleAdd}
          schemas={this.props.schemas}/>
        <div style={CompliancePage.STYLE.paddingLarge}/>
      </div>);
  }

  private onToggleAddRuleModal() {
    this.setState({isAddRuleModalOpen: !this.state.isAddRuleModalOpen});
  }

  private static readonly STYLE = {
    paddingMedium: {
      width: '100%',
      height: '20px'
    },
    paddingLarge: {
      width: '100%',
      height: '30px'
    },
    smallContent: {
      paddingTop: '18px',
      paddingLeft: '18px',
      paddingRight: '18px',
      paddingBottom: '60px',
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      flexBasis: '284px',
      flexGrow: 1,
      minWidth: '284px',
      maxWidth: '424px'
    },
    mediumContent: {
      paddingTop: '18px',
      paddingLeft: '18px',
      paddingRight: '18px',
      paddingBottom: '60px',
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      width: '732px'
    },
    largeContent: {
      paddingTop: '18px',
      paddingLeft: '18px',
      paddingRight: '18px',
      paddingBottom: '60px',
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      width: '1000px'
    }
  };
}
