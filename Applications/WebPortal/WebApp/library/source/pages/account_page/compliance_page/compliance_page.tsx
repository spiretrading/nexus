import { css, StyleSheet } from 'aphrodite/no-important';
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

  /** The page's model. */
  model: ComplianceModel;

  /** Whether an error occurred. */
  isError?: boolean;

  /** The status message to display. */
  status?: string;

  /** Determines if the component is readonly. */
  readonly?: boolean;

  /** The callback for adding the rule.*/
  onRuleAdd?: (newSchema: Nexus.ComplianceRuleSchema) => void;

  /** The callback for updating a changed rule. */
  onRuleChange?: (updatedRule: Nexus.ComplianceRuleEntry) => void;
}

interface State {
  isAddRuleModalOpen: boolean;
  isSubmitEnabled: boolean;
}

/** Displays the compliance page. */
export class CompliancePage extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    isError: false,
    status: '',
    onRuleAdd: () => {},
    onRuleChange: () => {}
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      isAddRuleModalOpen: false,
      isSubmitEnabled: false
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
    const footerStyle = (() => {
      if(this.props.readonly) {
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
          onChange={this.props.onRuleChange}
          readonly={this.props.readonly}/>
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
          <SubmissionInput roles={this.props.model.roles}
            isError={this.props.isError} status={this.props.status}
            isEnabled={this.state.isSubmitEnabled} onSubmit={this.onSubmit}/>
          <div style={CompliancePage.STYLE.paddingSmall}/>
          <div style={CompliancePage.STYLE.statusBox}>Saved</div>
        </div>
      </div>);
  }

  private onToggleAddRuleModal = () => {
    this.setState({isAddRuleModalOpen: !this.state.isAddRuleModalOpen});
  }

  private onSubmit = () => {
//    this.props.onSubmit(this.state.comment, this.state.parameters);
  }

  private static readonly STYLE = {
    paddingSmall: {
      width: '100%',
      height: '18px'
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
    },
    footer: {
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      flexWrap: 'nowrap' as 'nowrap',
      justifyContent: 'center' as 'center',
    },
    hidden: {
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    },
    statusBox: {
      height: '19px',
      width: '100%',
      display: 'flex' as 'flex',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      font: '400 14px Roboto',
      color: '#36BB55'
    }
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    button: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      width: '246px',
      backgroundColor: '#684BC7',
      color: '#FFFFFF',
      border: '0px solid #684BC7',
      borderRadius: '1px',
      font: '400 14px Roboto',
      outline: 'none',
      MozAppearance: 'none' as 'none',
      alignSelf: 'center' as 'center',
      cursor: 'pointer' as 'pointer',
      ':active' : {
        backgroundColor: '#4B23A0'
      },
      ':focus': {
        border: 0,
        outline: 'none',
        borderColor: '#4B23A0',
        backgroundColor: '#4B23A0',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none',
        MozAppearance: 'none' as 'none'
      },
      ':hover':{
        backgroundColor: '#4B23A0'
      },
      '::-moz-focus-inner': {
        border: 0,
        outline: 0
      },
      ':-moz-focusring': {
        outline: 0
      }
    }
  });
}
