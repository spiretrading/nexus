import { css, StyleSheet } from 'aphrodite/no-important';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Transition } from 'react-transition-group';
import { DisplaySize } from '../../..';
import { DropDownButton, HLine } from '../../..';
import { ParametersList } from './parameter_list';
import { RuleExecutionDropDown } from './rule_execution_drop_down';


interface Properties {
  
  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** The rule to display. */
  complianceRule: Nexus.ComplianceRuleEntry;

  /** The set of available currencies to select. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** Indicates if the component is readonly. */
  readonly?: boolean;

  /** The event handler called when the rule entry changes. */
  onChange?: (ruleEntry: Nexus.ComplianceRuleEntry) => void;
}

interface State {
  isExpanded: boolean;
  animationStyle: any;
}

/** Displays a given compliance rule. */
export class RuleRow extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    onChange: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      isExpanded: false,
      animationStyle: StyleSheet.create(this.applicabilityStyleDefinition)
    };
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
    const spacing = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return <div style={RuleRow.STYLE.mobilePadding}/>;
      } else {
        return null;
      }
    })();
    const headerTextStyle = (() => {
      if(this.state.isExpanded) {
        return RuleRow.STYLE.openText;
      } else {
        return RuleRow.STYLE.closedText;
      }
    })();
    const prefixPaddingStyle = RuleRow.STYLE.prefixPadding;
    const mode = {
      state: this.props.complianceRule.state,
      applicability: this.props.complianceRule.schema.applicability
    };
    return (
      <div style={RuleRow.STYLE.wrapper}>
        <div style={boxStyle}>
          {spacing}
          <div style={{...RuleRow.STYLE.prefix}}>
            <DropDownButton
              onClick={this.onRuleOpen}
              size={buttonSize}
              isExpanded={this.state.isExpanded}/>
            <div style={prefixPaddingStyle}/>
            <div style={headerTextStyle}>
              {this.props.complianceRule.schema.name}
            </div>
          </div>
          {spacing}
          <div style={RuleRow.STYLE.paddingLeft}>
            <RuleExecutionDropDown
              entryType={this.props.complianceRule.directoryEntry.type}
              displaySize={this.props.displaySize}
              value={mode}
              readonly={this.props.readonly}
              onChange={this.onRuleModeChange}/>
          </div>
          {spacing}
        </div>
          <Transition in={this.state.isExpanded}
              timeout={RuleRow.TRANSITION_LENGTH_MS}>
            {(state) => (
              <div ref={(divElement) => this.ruleParameters = divElement}
                  className={css((this.state.animationStyle as any)[state])}>
                <HLine color='#E6E6E6'/>
                <ParametersList 
                  displaySize={this.props.displaySize}
                  currencyDatabase={this.props.currencyDatabase}
                  schema={this.props.complianceRule.schema}
                  readonly={this.props.readonly}
                  onChange={this.onParameterChange}/>
              </div>)}
          </Transition>
      </div>);
  }

  public componentDidMount(): void {
    this.applicabilityStyleDefinition.entering.maxHeight =
      `${this.ruleParameters.scrollHeight}px`;
    this.applicabilityStyleDefinition.entered.maxHeight =
      `${this.ruleParameters.scrollHeight}px`;
    this.setState({
      animationStyle: StyleSheet.create(this.applicabilityStyleDefinition)
    });
  }

  private onRuleModeChange = (mode: RuleExecutionDropDown.Mode) => {
    const rule = new Nexus.ComplianceRuleEntry(
      this.props.complianceRule.id, this.props.complianceRule.directoryEntry,
      mode.state,
      this.props.complianceRule.schema.toApplicability(mode.applicability));
    this.props.onChange(rule);
  }

  private onRuleOpen = (event?: React.MouseEvent<any>) => {
    this.setState(state => {isExpanded: !state.isExpanded});
  }

  private onParameterChange = (schema: Nexus.ComplianceRuleSchema) => {
    const rule = new Nexus.ComplianceRuleEntry(
      this.props.complianceRule.id, this.props.complianceRule.directoryEntry,
      this.props.complianceRule.state, schema);
    this.props.onChange(rule);
  }

  private static readonly STYLE = {
    wrapper : {
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      width: '100%'
    },
    closedText: {
      font: '400 14px Roboto',
      color: '#333333',
      cursor: 'default' as 'default'
    },
    openText: {
      font: '500 14px Roboto',
      color: '#4B23A0',
      cursor: 'default' as 'default'
    },
    headerSmall: {
      minWidth: '284px',
      maxWidth: '424px',
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      height: '84px',
      font: '400 14px Roboto'
    },
    headerMedium: {
      width: '732px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      height: '54px',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between' as 'space-between',
      font: '400 14px Roboto',
    },
    headerLarge: {
      width: '1000px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      height: '54px',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between' as 'space-between',
      font: '400 14px Roboto',
    },
    prefix: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center'
    },
    paddingLeft: {
      paddingLeft: '38px',
      height: '34px',
      boxSizing: 'border-box' as 'border-box'
    },
    prefixPadding: {
      height: '20px',
      width: '18px',
    },
    mobilePadding: {
      height: '10px'
    }
  };
  private static readonly TRANSITION_LENGTH_MS = 600;
  private applicabilityStyleDefinition = {
    entering: {
      maxHeight: '0',
      overflow: 'hidden' as 'hidden',
      transitionProperty: 'max-height',
      transitionDuration: `${RuleRow.TRANSITION_LENGTH_MS}ms`
    },
    entered: {
      maxHeight: '0',
      overflow: 'hidden' as 'hidden'
    },
    exiting: {
      maxHeight: '0',
      overflow: 'hidden' as 'hidden',
      transitionProperty: 'max-height',
      transitionDuration: `${RuleRow.TRANSITION_LENGTH_MS}ms`
    },
    exited: {
      maxHeight: '0',
      overflow: 'hidden' as 'hidden'
    }
  };
  private ruleParameters: HTMLDivElement;
}
