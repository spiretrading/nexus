import { css, StyleSheet } from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../../display_size';
import { HLine } from '../../../components';

interface Properties {
  displaySize: DisplaySize;

  value?: Nexus.ComplianceValue[];
}

interface State {
  isEditing: boolean;
}

export class SecurityInput extends React.Component<Properties, State>{
  constructor(props: Properties) {
    super(props);
    this.state = {
      isEditing: false
    }
    this.toggleEditing = this.toggleEditing.bind(this);
  }

  public render() {
    const visibility = (() => {
      if(!this.state.isEditing) {
        return SecurityInput.STYLE.hidden;
      } else {
        return null;
      }
    })();
    const shadowBox = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SecurityInput.STYLE.boxShadowSmall;
      } else {
        return SecurityInput.STYLE.boxShadowBig;
      }
    })();
    const optionsBox = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SecurityInput.STYLE.smallOptionsBox;
      } else {
        return SecurityInput.STYLE.bigOptionsBox;
      }
    })();
    const selectedSecuritiesBox = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SecurityInput.STYLE.scrollBoxSmall;
      } else {
        return SecurityInput.STYLE.scrollBoxBig;
      }
    })();
    const iconRowStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SecurityInput.STYLE.iconRowSmall;
      } else {
        return SecurityInput.STYLE.iconRowBig;
      }
    })();
    const imageSize = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SecurityInput.IMAGE_SIZE_SMALL;
      } else {
        return SecurityInput.IMAGE_SIZE_BIG;
      }
    })();
    const imageWrapperStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SecurityInput.STYLE.iconWrapperSmall;
      } else {
        return SecurityInput.STYLE.iconWrapperLarge;
      }
    })();
    let displayValue  = '';
    const entries = [];
    for(let i = 0; i < this.props.value.length; ++i) {
      const sec = this.props.value[i].value as Nexus.Security;
      displayValue = displayValue.concat(sec.symbol.toString());
      entries.push();
      if(i >= 0 && i < this.props.value.length - 1 && this.props.value.length > 1) {
        displayValue = displayValue.concat(', ');
      }
    }
    
    return(
      <div>
        <input
          readOnly
          style={SecurityInput.STYLE.textBox}
          className={css(SecurityInput.EXTRA_STYLE.effects)}
          value ={displayValue}
          onFocus={this.toggleEditing.bind(this)}
          onClick={this.toggleEditing.bind(this)}/>
        <div style={visibility}>
          <div style={SecurityInput.STYLE.overlay}/>
          <div style={shadowBox}/>
          <div style={optionsBox}>
            <div style={SecurityInput.STYLE.header}>
              <div style={SecurityInput.STYLE.headerText}>
                {SecurityInput.MODAL_HEADER}
              </div>
              <img src={'resources/account_page/compliance_page/security_input/close.svg'}
                height='20px'
                width='20px'
                onClick={this.toggleEditing}/>
            </div>
            <input
              className={css(SecurityInput.EXTRA_STYLE.effects)}
              style={SecurityInput.STYLE.findSymbolBox}
              placeholder={SecurityInput.PLACEHOLDER_TEXT}
              value={''}/>
            <div style={selectedSecuritiesBox}>
              <div style={SecurityInput.STYLE.scrollBoxHeader}>
                {'Added Symbols'}
              </div>
            </div>
            <div style={iconRowStyle}>
              <div style={imageWrapperStyle}>
                <img src={'resources/account_page/compliance_page/security_input/remove-grey.svg'}/>
              </div>
              <div style={imageWrapperStyle}>
                <img src={'resources/account_page/compliance_page/security_input/upload.svg'}/>
              </div>
            </div>
            <HLine color={'#e6e6e6'}/>
            <div style={SecurityInput.STYLE.buttonWrapper}>
              <button style={SecurityInput.STYLE.button}>
                {'Submit Changes'}
              </button>
            </div>
          </div>
        </div>
      </div>);
  }

  private toggleEditing(){
    this.setState({isEditing: !this.state.isEditing});
  }

  private static readonly STYLE = {
    textBox: {
      textOverflow: 'ellipsis' as 'ellipsis',
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      font: '400 14px Roboto',
      color: '#000000',
      flexGrow: 1,
      width: '100%',
      paddingLeft: '10px',
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
    overlayMarginsBig: {
      width: '100%',
      height: '20px'
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
      height: '559px',
      top: 'calc(50% - 279.5px + 30px)',
      left: 'calc(50% - 180px)',
    },
    smallOptionsBox: {
      opacity: 1,
      boxSizing: 'border-box' as 'border-box',
      display: 'block',
      position: 'absolute' as 'absolute',
      border: '1px solid #FFFFFF',
      backgroundColor: '#FFFFFF',
      width: '282px',
      height: '100%',
      top: '0%',
      right: '0%',
      padding: '18px'
    },
    bigOptionsBox: {
      opacity: 1,
      boxSizing: 'border-box' as 'border-box',
      display: 'block',
      position: 'absolute' as 'absolute',
      backgroundColor: '#FFFFFF',
      width: '360px',
      height: '559px',
      top: 'calc(50% - 279.5px + 30px)',
      left: 'calc(50% - 180px)',
      padding: '18px',
    },
    header: {
      boxSizing: 'border-box' as 'border-box',
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      justifyContent: 'space-between' as 'space-between',
      height: '20px',
      marginBottom: '30px'
    },
    headerText: {
      font: '400 16px Roboto',
      flexGrow: 1
    },
    findSymbolBox: {
      width: '100%',
      boxSizing: 'border-box' as 'border-box',
      font: '400 14px Roboto',
      height: '34px',
      paddingLeft: '10px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      marginBottom: '18px'
    },
    scrollBoxSmall: {
      boxSizing: 'border-box' as 'border-box',
      height: '246px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      marginBottom: '30px'
    },
    scrollBoxBig: {
      boxSizing: 'border-box' as 'border-box',
      height: '280px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      marginBottom: '30px'
    },
    scrollBoxHeader: {
      boxSizing: 'border-box' as 'border-box',
      height: '40px',
      width: '100%',
      backgroundColor: '#F8F8F8',
      font: '400 14px Roboto',
      paddingLeft: '10px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center'
    },
    scrollBoxEntry: {
    },
    scrollBoxEntrySelected: {
    },
    iconWrapperSmall: {
      height: '24px',
      width: '24px'
    },
    iconWrapperLarge: {
      height: '16px',
      width: '16px'
    },
    iconRowSmall: {
      height: '24px',
      width: '100%',
      marginBottom: '30px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      justifyContent: 'space-evenly' as 'space-evenly' 
    },
    iconRowBig: {
      height: '16px',
      width: '100%',
      marginBottom: '30px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      justifyContent: 'space-evenly' as 'space-evenly' 
    },
    button: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      width: '246px',
      backgroundColor: '#4B23A0',
      color: '#FFFFFF',
      border: '1px solid #4B23A0',
      borderRadius: '1px',
      marginTop: '30px',
      font: '400 16px Roboto',
    },
    buttonWrapper: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'wrap' as 'wrap',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center',
    }
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    effects: {
      ':focus': {
        ouline: 0,
        borderColor: '#684BC7',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      ':active' : {
        borderColor: '#684BC7'
      },
      '::moz-focus-inner': {
        border: 0
      },
      '::placeholder': {
        color: '#8C8C8C'
      }
    }
  });
  private static readonly MODAL_HEADER = 'Edit Symbols';
  private static readonly PLACEHOLDER_TEXT = 'Find symbol here';
  private static readonly PATH =
    'resources/account_page/compliance_page/security_input/';
  private static readonly IMAGE_SIZE_SMALL = '20px';
  private static readonly IMAGE_SIZE_BIG = '16px';
}