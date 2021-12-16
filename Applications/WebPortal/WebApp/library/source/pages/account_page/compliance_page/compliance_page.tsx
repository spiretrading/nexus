import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, HLine } from '../../../';
import { SubmissionInput } from '..';
import { ComplianceModel } from './compliance_model';
import { NewRuleModal } from './new_rule_modal';
import { RulesList } from './rules_list';

interface Properties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** The roles belonging to the account viewing this page. */
  roles: Nexus.AccountRoles;

  /** The page's model. */
  model: ComplianceModel;

  /** Whether an error occurred. */
  isError?: boolean;

  /** The status message to display. */
  status?: string;

  /** Whether the changes can be submitted. */
  canSubmit?: boolean;

  /** The callback for adding the rule.*/
  onRuleAdd?: (newSchema: Nexus.ComplianceRuleSchema) => void;

  /** The callback for updating a changed rule. */
  onRuleChange?: (updatedRule: Nexus.ComplianceRuleEntry) => void;

  /** Indicates the compliance rules are to be submitted. */
  onSubmit?: () => void;
}

interface State {
  isAddRuleModalOpen: boolean;
}

/** Displays the compliance page. */
export class CompliancePage extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    isError: false,
    status: '',
    canSubmit: false,
    onRuleAdd: () => {},
    onRuleChange: () => {}
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      isAddRuleModalOpen: false
    };
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
    const readonly =
      !this.props.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR);
    const footerStyle = (() => {
      if(readonly) {
        return CompliancePage.STYLE.hidden;
      } else {
        return CompliancePage.STYLE.footer;
      }
    })();
    return (
      <div style={contentStyle}>
        <RulesList
          displaySize={this.props.displaySize}
          currencyDatabase={this.props.model.currencyDatabase}
          complianceList={this.props.model.entries}
          onChange={this.props.onRuleChange} readonly={readonly}/>
        <div style={footerStyle}>
          <div style={CompliancePage.STYLE.paddingMedium}/>
          <NewRuleModal displaySize={this.props.displaySize}
            isOpen={this.state.isAddRuleModalOpen}
            onToggleModal={this.onToggleAddRuleModal}
            onAddNewRule={this.props.onRuleAdd}
            schemas={this.props.model.schemas}/>
          <div style={CompliancePage.STYLE.paddingLarge}/>
          <HLine color='#E6E6E6'/>
          <div style={CompliancePage.STYLE.paddingLarge}/>
          <SubmissionInput roles={this.props.roles}
            isError={this.props.isError} status={this.props.status}
            isEnabled={this.props.canSubmit} onSubmit={this.props.onSubmit}/>
        </div>
      </div>);
  }

  private onToggleAddRuleModal = () => {
    this.setState({isAddRuleModalOpen: !this.state.isAddRuleModalOpen});
  }

  private static readonly STYLE = {
    paddingSmall: {
      width: '100%',
      height: '18px'
    } as React.CSSProperties,
    paddingMedium: {
      width: '100%',
      height: '20px'
    } as React.CSSProperties,
    paddingLarge: {
      width: '100%',
      height: '30px'
    } as React.CSSProperties,
    smallContent: {
      paddingTop: '18px',
      paddingLeft: '18px',
      paddingRight: '18px',
      paddingBottom: '60px',
      display: 'flex',
      flexDirection: 'column',
      flexBasis: '284px',
      flexGrow: 1,
      minWidth: '284px',
      maxWidth: '424px'
    } as React.CSSProperties,
    mediumContent: {
      paddingTop: '18px',
      paddingLeft: '18px',
      paddingRight: '18px',
      paddingBottom: '60px',
      display: 'flex',
      flexDirection: 'column',
      width: '732px'
    } as React.CSSProperties,
    largeContent: {
      paddingTop: '18px',
      paddingLeft: '18px',
      paddingRight: '18px',
      paddingBottom: '60px',
      display: 'flex',
      flexDirection: 'column',
      width: '1000px'
    } as React.CSSProperties,
    footer: {
      display: 'flex',
      flexDirection: 'column',
      flexWrap: 'nowrap',
      justifyContent: 'center',
    } as React.CSSProperties,
    hidden: {
      visibility: 'hidden',
      display: 'none'
    } as React.CSSProperties,
    statusBox: {
      height: '19px',
      width: '100%',
      display: 'flex',
      alignItems: 'center',
      justifyContent: 'center',
      font: '400 14px Roboto',
      color: '#36BB55'
    } as React.CSSProperties
  };
}
