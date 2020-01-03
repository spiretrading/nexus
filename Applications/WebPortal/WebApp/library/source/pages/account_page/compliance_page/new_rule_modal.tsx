import { css, StyleSheet } from 'aphrodite/no-important';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize,  HLine } from '../../..';

interface Properties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** The list of rule schemas that define the new rules. */
  schemas: Nexus.ComplianceRuleSchema[];

  /** Determines if the modal should be open or not. */
  isOpen?: boolean;

  /** The callback to hide or show the uploader. */
  onToggleModal?: () => void;

  /** The callback to add a rule with the following schema. */
  onAddNewRule?: (newRule: Nexus.ComplianceRuleSchema) => void;
}

interface State {
  selection: number;
}

/** Displays a component that allows a user to add a new rule. */
export class NewRuleButton extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    isOpen: false,
    onToggleModal: () => {},
    onAddNewRule: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      selection: -1
    }
  }

  public render(): JSX.Element {
    const contentBoxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return NewRuleButton.STYLE.smallOptionsBox;
      } else {
        return NewRuleButton.STYLE.largeOptionsBox;
      }
    })();
    const shadowBoxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return NewRuleButton.STYLE.boxShadowSmall;
      } else {
        return NewRuleButton.STYLE.boxShadowLarge;
      }
    })();
    const modalStyle = (() => {
      if(this.props.isOpen) {
        return null;
      } else {
        return NewRuleButton.STYLE.hidden;
      }
    })();
    const plusSignImageSize = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return '20px';
      } else {
        return '16px';
      }
    })();
    const buttonWrapper = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return NewRuleButton.STYLE.buttonWrapperSmall;
      } else {
        return NewRuleButton.STYLE.buttonWrapper;
      }
    })();
    const options = [];
    for(let i = 0; i < this.props.schemas.length; ++i) {
      if(i === this.state.selection) {
        options.push(
          <div className={css(NewRuleButton.EXTRA_STYLE.selectedRow)}
              onClick={this.onClickRule.bind(this, i)}>
            {this.props.schemas[i].name}
          </div>);
      } else {
        options.push(
          <div className={css(NewRuleButton.EXTRA_STYLE.optionRow)}
            onClick={this.onClickRule.bind(this, i)}>
            {this.props.schemas[i].name}
          </div>);
      }
    }
    return (
      <div>
        <div style={NewRuleButton.STYLE.newRuleRow}
            onClick={this.props.onToggleModal}>
          <div style={NewRuleButton.STYLE.imageWrapper}>
            <img src='resources/account_page/compliance_page/add.svg'
              height={plusSignImageSize}
              width={plusSignImageSize}/>
          </div>
          <div style={NewRuleButton.STYLE.newRuleText}>{'Add New Rule'}</div>
        </div>
        <div style={modalStyle}>
          <div style={NewRuleButton.STYLE.overlay}/>
          <div style={shadowBoxStyle}/>
          <div style={contentBoxStyle}>
            <div style={NewRuleButton.STYLE.header}>
              <div style={NewRuleButton.STYLE.headerText}>
                {'Add New Rule'}
              </div>
              <img height={plusSignImageSize} width={plusSignImageSize}
                onClick={this.props.onToggleModal}
                src='resources/account_page/compliance_page/new_row_modal/remove.svg'/>
            </div>
            <div style={NewRuleButton.STYLE.ruleItemWrapper}>
              {options}
            </div>
            <div style={NewRuleButton.STYLE.footerWrapper}>
              <HLine color='#E6E6E6'/>
              <div style={buttonWrapper}>
                <button 
                    className={css(NewRuleButton.EXTRA_STYLE.button)}
                    onClick={this.addNewRule.bind(this)}>
                  {'Select'}
                </button>
              </div>
            </div>
          </div>
        </div>
      </div>);
  }

  private onClickRule(index: number) {
    if(index === this.state.selection) {
      this.setState({selection: -1});
    } else {
      this.setState({selection: index});
    }
  }

  private addNewRule() {
    if(this.state.selection === -1) {
      return;
    }
    this.props.onAddNewRule(this.props.schemas[this.state.selection]);
    this.props.onToggleModal();
    this.setState({selection: -1});
  }

  private static readonly STYLE = {
    newRuleRow: {
      height: '20px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center'
    },
    newRuleText: {
      font: '400 14px Roboto',
      paddingLeft: '18px',
      color: '#333333'
    },
    imageWrapper: {
      display: 'flex' as 'flex',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center',
      width: '20px',
      height: '20px',
      cursor: 'pointer'
    },
    hidden: {
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    },
    overlay: {
      boxSizing: 'border-box' as 'border-box',
      top: '0px',
      left: '0px',
      position: 'fixed' as 'fixed',
      width: '100%',
      height: '100%',
      minHeight: '501px',
      backgroundColor: '#FFFFFF',
      opacity: 0.9
    },
    boxShadowSmall: {
      boxSizing: 'border-box' as 'border-box', 
      opacity: 0.4,
      display: 'block',
      boxShadow: '0px 0px 6px #000000',
      position: 'absolute' as 'absolute',
      border: '1px solid #FFFFFF',
      backgroundColor: '#FFFFFF',
      width: '282px',
      height: '100%',
      top: '0%',
      right: '0%'
    },
    boxShadowLarge: {
      boxSizing: 'border-box' as 'border-box',
      opacity: 0.4,
      boxShadow: '0px 0px 6px #000000',
      display: 'block',
      position: 'absolute' as 'absolute',
      backgroundColor: '#FFFFFF',
      width: '360px',
      height: '461px',
      top: 'calc(50% - 230.5px)',
      left: 'calc(50% - 180px)',
    },
    smallOptionsBox: {
      opacity: 1,
      boxSizing: 'border-box' as 'border-box',
      display: 'block',
      position: 'absolute' as 'absolute',
      backgroundColor: '#FFFFFF',
      width: '282px',
      height: '100%',
      top: '0%',
      right: '0%'
    },
    largeOptionsBox: {
      opacity: 1,
      boxSizing: 'border-box' as 'border-box',
      display: 'block',
      position: 'absolute' as 'absolute',
      backgroundColor: '#FFFFFF',
      width: '360px',
      height: '461px',
      top: 'calc(50% - 230.5px)',
      left: 'calc(50% - 180px)'
    },
    header: {
      boxSizing: 'border-box' as 'border-box',
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      justifyContent: 'space-between' as 'space-between',
      height: '38px',
      paddingLeft: '18px',
      paddingRight: '18px',
      paddingTop: '18px'
    },
    footerWrapper: {
      boxSizing: 'border-box' as 'border-box',
      paddingLeft: '18px',
      paddingRight: '18px'
    },
    headerText: {
      font: '400 16px Roboto',
      height: '20px',
    },
    iconWrapperSmall: {
      height: '24px',
      width: '24px'
    },
    iconWrapperLarge: {
      height: '16px',
      width: '16px'
    },
    ruleItemWrapper: {
      paddingTop: '30px',
      paddingBottom: '30px'
    },
    buttonWrapperSmall: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'wrap' as 'wrap',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center',
      paddingTop: '30px',
      paddingBottom: '40px'
    },
    buttonWrapper: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'wrap' as 'wrap',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center',
      paddingTop: '30px',
      paddingBottom: '18px'
    }
  };

  private static readonly EXTRA_STYLE = StyleSheet.create({
    optionRow: {
      boxSizing: 'border-box' as 'border-box',
      height: '40px',
      width: '100%',
      font: '400 14px Roboto',
      color: '#333333',
      display: 'flex' as 'flex',
      alignItems: 'center' as 'center',
      paddingLeft: '18px',
      ':hover': {
        backgroundColor: '#F8F8F8'
      }
    },
    selectedRow: {
      boxSizing: 'border-box' as 'border-box',
      height: '40px',
      width: '100%',
      font: '400 14px Roboto',
      color: '#FFFFFF',
      backgroundColor: '#684BC7',
      display: 'flex' as 'flex',
      alignItems: 'center' as 'center',
      paddingLeft: '18px'
    },
    button: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      width: '246px',
      backgroundColor: '#684BC7',
      color: '#FFFFFF',
      border: '0px solid #684BC7',
      borderRadius: '1px',
      font: '400 16px Roboto',
      outline: 'none',
      MozAppearance: 'none' as 'none',
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
