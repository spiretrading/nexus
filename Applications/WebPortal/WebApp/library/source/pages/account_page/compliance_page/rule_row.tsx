import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { Transition } from 'react-transition-group';
import { DisplaySize } from '../../../display_size';
import { DropDownButton, HLine} from '../../../components';
import { RuleExecutionDropDown, RuleMode } from './rule_execution_drop_down';
import { RuleParameters } from './parameter_list';

interface Properties {
  
  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;
  //on update
}

interface State {
  ruleMode: RuleMode;
  isExpanded: boolean;
  animationStyle: any;
}

export class RuleRow extends React.Component<Properties, State> {
  constructor(props: Properties){
    super(props);
    this.state = {
      ruleMode: RuleMode.PASSIVE,
      isExpanded: false,
      animationStyle: StyleSheet.create(this.applicabilityStyleDefinition)
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
    return(
      <div style={RuleRow.STYLE.wrapper}>
        <div style={boxStyle}>
          {spacing}
          <div style={{...RuleRow.STYLE.prefix}}>
            <DropDownButton
              onClick={this.onRuleOpen}
              size={buttonSize}
              isExpanded={this.state.isExpanded}/>
            <div style={prefixPaddingStyle}/>
            <div style={headerTextStyle}>{'Some Rule'}</div>
          </div>
          {spacing}
          <div style={RuleRow.STYLE.paddingLeft}>
            <RuleExecutionDropDown
              onChange={this.onRuleModeChange}
              value={this.state.ruleMode}
              displaySize={this.props.displaySize}/>
          </div>
          {spacing}
        </div>
          <Transition in={this.state.isExpanded}
              timeout={RuleRow.TRANSITION_LENGTH_MS}>
            {(state) => (
              <div ref={(divElement) => this.ruleParameters = divElement}
                  className={css((this.state.animationStyle as any)[state])}>
                <HLine color='#E6E6E6'/>
                <RuleParameters displaySize={this.props.displaySize}/>
              </div>)}
          </Transition>


        {
        //{line}
        //<RuleParameters displaySize={this.props.displaySize}/>
        }
      </div>);
  }

  public componentDidMount(): void {
    this.applicabilityStyleDefinition.entering.maxHeight =
      `${this.ruleParameters.offsetHeight}px`;
    this.applicabilityStyleDefinition.entered.maxHeight =
      `${this.ruleParameters.offsetHeight}px`;
    this.setState({
      animationStyle: StyleSheet.create(this.applicabilityStyleDefinition)
    });
  }

  private onRuleModeChange(newMode: RuleMode) {
    this.setState({ruleMode: newMode});
  }

  private onRuleOpen(event?: React.MouseEvent<any>) {
    this.setState({isExpanded: !this.state.isExpanded});
  }

  private static readonly STYLE = {
    wrapper : {
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      width: '100%'
    },
    closedText: {
      font: '400 14px Roboto',
      color: '#333333'
    },
    openText: {
      font: '500 14px Roboto',
      color: '#4B23A0'
    },
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
  private static readonly TRANSITION_LENGTH_MS = 600;
  private static readonly MOBILE_BUTTON_SIZE_PX = '20px';
  private static readonly DESKTOP_BUTTON_SIZE_PX = '16px';
  private ruleParameters: HTMLDivElement;
}
