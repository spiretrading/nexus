import { css, StyleSheet } from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../../display_size';
import { HLine, Modal } from '../../../components';

interface Properties {
  displaySize: DisplaySize;

  value?: Nexus.ComplianceValue[];

  onChange?: (newValue: Nexus.ComplianceValue[]) => void;
}

interface State {
  isEditing: boolean;
  selection: number;
  inputString: string;
}

export class SecurityInput extends React.Component<Properties, State>{
  constructor(props: Properties) {
    super(props);
    this.state = {
      isEditing: false,
      selection: -1,
      inputString: ''
    }
    this.toggleEditing = this.toggleEditing.bind(this);
    this.onInputChange = this.onInputChange.bind(this);
    this.addEntry = this.addEntry.bind(this);
  }

  public render() {
    const visibility = (() => {
      if(!this.state.isEditing) {
        return SecurityInput.STYLE.hidden;
      } else {
        return null;
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
    const removeImgSrc = (() => {
      if(this.state.selection !== -1) {
        return (
          <div style={imageWrapperStyle} onClick={this.removeEntry.bind(this)}>
            <img src={'resources/account_page/compliance_page/security_input/remove-purple.svg'}/>
          </div>);
      } else {
        return  (
          <div style={imageWrapperStyle}>
            <img src={'resources/account_page/compliance_page/security_input/remove-grey.svg'}/>
          </div>);
      }
    })();
    let displayValue  = '';
    const entries = [];
    for(let i = 0; i < this.props.value.length; ++i) {
      const sec = this.props.value[i].value as Nexus.Security;
      displayValue = displayValue.concat(sec.symbol.toString());
      if(this.state.selection === i) {
        entries.push(
          <div style={SecurityInput.STYLE.scrollBoxEntrySelected}
              onClick={this.selectEntry.bind(this, i)}>
            {sec.symbol.toString()}
          </div>);
      } else {
        entries.push(
          <div style={SecurityInput.STYLE.scrollBoxEntry}
              onClick={this.selectEntry.bind(this, i)}>
            {sec.symbol.toString()}
          </div>);
      }
      if(i >= 0 && i < this.props.value.length - 1 && this.props.value.length > 1) {
        displayValue = displayValue.concat(', ');
      }
    }
    
    return(
      <div>
        <input
          style={SecurityInput.STYLE.textBox}
          className={css(SecurityInput.EXTRA_STYLE.effects)}
          value ={displayValue}
          onFocus={this.toggleEditing.bind(this)}
          onClick={this.toggleEditing.bind(this)}/>
        <div style={visibility}>
          <Modal displaySize={this.props.displaySize} 
              height={'559px'} width={'360px'}>
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
                onChange={this.onInputChange}
                onKeyDown={this.addEntry}
                value={this.state.inputString}/>
              <div style={selectedSecuritiesBox}>
                <div style={SecurityInput.STYLE.scrollBoxHeader}>
                  {'Added Symbols'}
                </div>
                {entries}
              </div>
              <div style={iconRowStyle}>
                {removeImgSrc}
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
          </Modal>
          </div>
        </div>);
  }

  private toggleEditing(){
    this.setState({isEditing: !this.state.isEditing});
  }

  private selectEntry(index: number) {
    if(index === this.state.selection) {
      this.setState({selection: -1});
    } else {
      this.setState({selection: index});
    }
  }

  private removeEntry() {
    if(this.state.selection !== -1) {
      this.props.onChange(
        this.props.value.slice(0, this.state.selection).concat(
        this.props.value.slice(this.state.selection+1)));
    }
    this.setState({selection: -1});
  }

  private onInputChange(event: React.ChangeEvent<HTMLInputElement>) {
    this.setState({inputString: event.target.value});
  }

  private addEntry(event: React.KeyboardEvent<HTMLInputElement>) {
    if(event.keyCode === 13) {
      const newParameter = 
        new Nexus.ComplianceValue( 
          Nexus.ComplianceValue.Type.SECURITY, 
          new Nexus.Security(
            this.state.inputString,
            Nexus.MarketCode.NONE,
            Nexus.DefaultCountries.CA));
      this.props.onChange(
        this.props.value.slice().concat(newParameter));
      this.setState({inputString: ''});
    }
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
      marginBottom: '30px',
      overflowY: 'scroll' as 'scroll'
    },
    scrollBoxBig: {
      boxSizing: 'border-box' as 'border-box',
      height: '280px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      marginBottom: '30px',
      overflowY: 'scroll' as 'scroll'
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
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      width: '100%',
      backgroundColor: '#FFFFFF',
      color: '#000000',
      font: '400 14px Roboto',
      paddingLeft: '10px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center'
    },
    scrollBoxEntrySelected: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      width: '100%',
      backgroundColor: '#684BC7',
      color: '#FFFFFF',
      font: '400 14px Roboto',
      paddingLeft: '10px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center'
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