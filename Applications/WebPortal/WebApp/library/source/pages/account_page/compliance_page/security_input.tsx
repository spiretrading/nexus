import { css, StyleSheet } from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, HLine } from '../../..';

interface Properties {
  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** The size at which the component should be displayed at. */
  value?: Nexus.ComplianceValue[];

  /** Whether the selection box is read only. */
  readonly?: boolean;

  /** Called when the value changes.
   * @param newValue - The updated value.
   */
  onChange?: (newValue: Nexus.ComplianceValue[]) => void;
}

interface State {
  isEditing: boolean;
  selection: number;
  inputString: string;
}

/** A component that displays a list of securities. */
export class SecurityInput extends React.Component<Properties, State>{
  public static readonly defaultProps = {
    value: '',
    readonly: false,
    onChange: () => {}
  }

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
        return SecurityInput.STYLE.modal;
      }
    })();
    const shadowBox = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SecurityInput.STYLE.boxShadowSmall;
      } else {
        if(this.props.readonly) {
          return SecurityInput.STYLE.boxShadowBigReadonly;
        } else {
          return SecurityInput.STYLE.boxShadowBig;
        }
      }
    })();
    const modalBox = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SecurityInput.STYLE.smallModalBox;
      } else {
        if(this.props.readonly) {
          return SecurityInput.STYLE.bigModalBoxReadonly;
        } else {
          return SecurityInput.STYLE.bigModalBox;
        }
      }
    })();
    const findSymbolBox = (() => {
      if(this.props.readonly) {
        return SecurityInput.STYLE.hidden;
      } else {
        return SecurityInput.STYLE.findSymbolBox;
      }
    })();
    const selectedSecuritiesBox = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        if(this.props.readonly) {
          return SecurityInput.STYLE.scrollBoxSmallReadonly;
        } else {
          return SecurityInput.STYLE.scrollBoxSmall;
        }
      } else {
        return SecurityInput.STYLE.scrollBoxBig;
      }
    })();
    const iconRowStyle = (() => {
      if(this.props.readonly) {
        return SecurityInput.STYLE.hidden;
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return SecurityInput.STYLE.iconRowSmall;
        } else {
          return SecurityInput.STYLE.iconRowBig;
        }
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
            <img src={SecurityInput.PATH + 'remove-purple.svg'}/>
          </div>);
      } else {
        return (
          <div style={imageWrapperStyle}>
            <img src={SecurityInput.PATH + 'remove-grey.svg'}/>
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
      if(i >= 0 && i < this.props.value.length - 1 && 
          this.props.value.length > 1) {
        displayValue = displayValue.concat(', ');
      }
    }
    
    return(
      <div>
        <input
          style={SecurityInput.STYLE.textBox}
          className={css(SecurityInput.EXTRA_STYLE.effects)}
          value={displayValue}
          onFocus={this.toggleEditing.bind(this)}
          onClick={this.toggleEditing.bind(this)}/>
        <div style={visibility}>
          <div style={SecurityInput.STYLE.overlay}/>
          <div style={shadowBox}/>
          <div style={modalBox}>
            <div style={SecurityInput.STYLE.header}>
              <div style={SecurityInput.STYLE.headerText}>
                {SecurityInput.MODAL_HEADER}
              </div>
              <img src={SecurityInput.PATH + 'close.svg'}
                height='20px'
                width='20px'
                onClick={this.toggleEditing}/>
            </div>
            <input
              className={css(SecurityInput.EXTRA_STYLE.effects)}
              style={findSymbolBox}
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
                <img src={SecurityInput.PATH + 'upload.svg'}/>
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
    if(!this.props.readonly) {
      this.setState({isEditing: !this.state.isEditing});
    }
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
      color: '#333333',
      flexGrow: 1,
      width: '100%',
      paddingLeft: '10px',
      cursor: 'pointer' as 'pointer'
    },
    hidden: {
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    },
    modal: {
      boxSizing: 'border-box' as 'border-box',
      minHeight: '559px',
      overflow: 'scroll',
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
      width: '300px',
      height: '559px',
      top: 'calc(50% - 279.5px + 30px)',
      left: 'calc(50% - 180px)',
    },
    boxShadowBigReadonly: {
      boxSizing: 'border-box' as 'border-box',
      opacity: 0.4,
      boxShadow: '0px 0px 6px #000000',
      display: 'block',
      position: 'absolute' as 'absolute',
      backgroundColor: '#FFFFFF',
      width: '300px',
      height: '424px',
      top: 'calc(50% - 212px + 30px)',
      left: 'calc(50% - 180px)',
    },
    smallModalBox: {
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
      paddingLeft: '18px',
      paddingRight: '18px',
      paddingTop: '18px',
      paddingBottom: '40px',
      overflowY: 'auto' as 'auto'
    },
    bigModalBox: {
      opacity: 1,
      boxSizing: 'border-box' as 'border-box',
      display: 'block',
      position: 'absolute' as 'absolute',
      backgroundColor: '#FFFFFF',
      width: '300px',
      height: '559px',
      top: 'calc(50% - 279.5px + 30px)',
      left: 'calc(50% - 180px)',
      padding: '18px',
    },
    bigModalBoxReadonly: {
      opacity: 1,
      boxSizing: 'border-box' as 'border-box',
      display: 'block',
      position: 'absolute' as 'absolute',
      backgroundColor: '#FFFFFF',
      width: '300px',
      height: '424px',
      top: 'calc(50% - 212px + 30px)',
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
      overflowY: 'scroll' as 'scroll'
    },
    scrollBoxSmallReadonly: {
      boxSizing: 'border-box' as 'border-box',
      height: '342px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'scroll' as 'scroll'
    },
    scrollBoxBig: {
      boxSizing: 'border-box' as 'border-box',
      height: '280px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
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
      marginTop: '30px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      justifyContent: 'space-evenly' as 'space-evenly' 
    },
    iconRowBig: {
      height: '16px',
      width: '100%',
      marginBottom: '30px',
      marginTop: '30px',
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
        borderColor: '#684BC7',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
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
  private static readonly SUBMIT_CHANGES = ''; //???
  private static readonly CONFIRM = ''; //???
  private static readonly PATH =
    'resources/account_page/compliance_page/security_input/';
  private static readonly IMAGE_SIZE_SMALL = '20px';
  private static readonly IMAGE_SIZE_BIG = '16px';
}