import { css, StyleSheet } from 'aphrodite/no-important';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../../display_size';
import { HLine } from '../../../components';

interface Properties {
  
  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** A list of rule Schemas */
  ruleSchemas: Nexus.ComplianceRuleSchema[];
}

interface State {
  /** Indicates if the modal should be open ot not. */
  isOpen: boolean;

  selection: number;
}

/** Displays a component that allows a user to add a new rule. */
export class NewRuleButton extends React.Component<Properties, State> {
  public constructor(props: Properties) {
    super(props);
    this.state = {
      isOpen: false,
      selection: -1
    }
  }

  public render(): JSX.Element {
    const contentBoxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return NewRuleButton.STYLE.smallOptionsBox;
      } else {
        return NewRuleButton.STYLE.bigOptionsBox;
      }
    })();
    const shadowBoxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return NewRuleButton.STYLE.boxShadowSmall;
      } else {
        return NewRuleButton.STYLE.boxShadowBig;
      }
    })();
    const modalStyle = (() => {
      if(this.state.isOpen) {
        return null;
      } else {
        return NewRuleButton.STYLE.hidden;
      }
    })();
    const imageSize = (() => {
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
    for(let i = 0; i < this.props.ruleSchemas.length; ++i) {
        if(i === this.state.selection) {
          options.push(
            <div className={css(NewRuleButton.EXTRA_STYLE.selectedRow)}
                onClick={this.onClickRule.bind(this, i)}>
              {this.props.ruleSchemas[i].name}
            </div>);
        } else {
          options.push(
            <div className={css(NewRuleButton.EXTRA_STYLE.optionRow)}
              onClick={this.onClickRule.bind(this, i)}>
              {this.props.ruleSchemas[i].name}
            </div>);
      }
    }
    return (
      <div>
        <div style={NewRuleButton.STYLE.newRuleRow}
            onClick={this.onClickClose.bind(this)}>
          <div style={NewRuleButton.STYLE.imageWrapper}>
            <img src='resources/account_page/compliance_page/add.svg'
              height={imageSize}
              width={imageSize}/>
          </div>
          <div style={NewRuleButton.STYLE.newRuleText}>Add New Rule</div>
        </div>
        <div style={modalStyle}>
          <div style={NewRuleButton.STYLE.overlay}/>
          <div style={shadowBoxStyle}/>
          <div style={contentBoxStyle}>
            <div style={NewRuleButton.STYLE.header}>
              <div style={NewRuleButton.STYLE.headerText}>
                {NewRuleButton.MODAL_HEADER}
              </div>
              <img height={imageSize} width={imageSize}
                src='resources/account_page/compliance_page/new_row_modal/remove.svg'/>
            </div>
            <div style={NewRuleButton.STYLE.ruleItemWraper}>
              {options}
            </div>
            <div style={NewRuleButton.STYLE.footerWrapper}>
              <HLine color='#E6E6E6'/>
              <div style={buttonWrapper}>
                <button 
                    className={css(NewRuleButton.EXTRA_STYLE.bacon)}
                    onClick={()=>{}}>
                  {NewRuleButton.BUTTON_TEXT}
                </button>
              </div>
            </div>
          </div>
        </div>
      </div>);
  }

  private onClickClose() {
    this.setState({isOpen: !this.state.isOpen});
  }

  private onClickRule(index: number) {
    if(index === this.state.selection) {
      this.setState({selection: -1});
    } else {
      this.setState({selection: index});
    }
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
      paddingLeft: '18px'
    },
    imageWrapper: {
      position: 'relative' as 'relative',
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
    boxShadowBig: {
      boxSizing: 'border-box' as 'border-box',
      opacity: 0.4,
      boxShadow: '0px 0px 6px #000000',
      display: 'block',
      position: 'absolute' as 'absolute',
      backgroundColor: '#FFFFFF',
      width: '360px',
      height: '461px',
      top: 'calc(50% - 279.5px)',
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
    bigOptionsBox: {
      opacity: 1,
      boxSizing: 'border-box' as 'border-box',
      display: 'block',
      position: 'absolute' as 'absolute',
      backgroundColor: '#FFFFFF',
      width: '360px',
      height: '461px',
      top: 'calc(50% - 279.5px)',
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
    ruleItemWraper: {
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
      paddingBottom: '18px',
    }
  };

  private static readonly EXTRA_STYLE = StyleSheet.create({
    optionRow: {
      boxSizing: 'border-box' as 'border-box',
      height: '40px',
      width: '100%',
      font: '400 14px Roboto',
      color: '#000000',
      display: 'flex' as 'flex',
      alignItems: 'center' as 'center',
      paddingLeft: '18px',
      ':hover': {
        backgroundColor: '#F8F8F8',
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
      paddingLeft: '18px',
    },
    bacon: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      width: '246px',
      backgroundColor: '#684BC7',
      color: '#FFFFFF',
      border: '0px solid #684BC7',
      borderRadius: '1px',
      font: '400 16px Roboto',
      ':active' : {
        backgroundColor: 'red'
      },
      ':focus': {
        ouline: 0,
        borderColor: '#4B23A0',
        backgroundColor: '#4B23A0',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      ':hover':{
        backgroundColor: '#4B23A0'
      },
      '::moz-focus-inner': {
        border: 0
      },
      '::placeholder': {
        color: '#8C8C8C'
      }
    }
  });
  private static readonly MODAL_HEADER = 'Add New Rule';
  private static readonly BUTTON_TEXT = 'Select';
  private static readonly IMAGE_SIZE_SMALL = '20px';
  private static readonly IMAGE_SIZE_BIG = '16px';
}
