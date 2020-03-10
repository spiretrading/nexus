import { css, StyleSheet } from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Button, DisplaySize, HLine, Modal } from '..';

interface Properties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** Determines if the component is readonly. */
  readonly?: boolean;

  /** The list of securities to display. */
  value?: Nexus.Security;

  /** Called when the list of values changes.
   * @param value - The new security
   */
  onChange?: (value: Nexus.Security) => void;
}

interface State {
  inputString: string;
  isEditing: boolean;
  localValue: Nexus.Security;
  isSelected: boolean;
}

/** A component that displays a list of securities. */
export class SingleSecurityInput extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    value: '',
    readonly: false,
    onChange: () => {}
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      inputString: '',
      isEditing: false,
      localValue: this.props.value,
      isSelected: false
    }
    this.addEntry = this.addEntry.bind(this);
    this.onInputChange = this.onInputChange.bind(this);
    this.onSubmitChange = this.onSubmitChange.bind(this);
    this.removeEntry = this.removeEntry.bind(this);
    this.selectEntry = this.selectEntry.bind(this);
    this.onClose = this.onClose.bind(this);
    this.onOpen = this.onOpen.bind(this);
  }

  public render() {
    const visibility = (() => {
      if(!this.state.isEditing) {
        return SingleSecurityInput.STYLE.hidden;
      } else {
        return null;
      }
    })();
    const inputField = (() => {
      return (
        <InputField
          value={this.state.inputString}
          onChange={this.onInputChange}
          onEnter={this.addEntry}/>);
    })();
    const iconRowStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SingleSecurityInput.STYLE.iconRowSmall;
      } else {
        return SingleSecurityInput.STYLE.iconRowLarge;
      }
    })();
    const imageSize = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SingleSecurityInput.IMAGE_SIZE_SMALL_VIEWPORT;
      } else {
        return SingleSecurityInput.IMAGE_SIZE_LARGE_VIEWPORT;
      }
    })();
    const iconWrapperStyle = (() => {
      const displaySize = this.props.displaySize;
      if(displaySize === DisplaySize.SMALL) {
        return SingleSecurityInput.STYLE.iconWrapperSmall;
      } else {
        return SingleSecurityInput.STYLE.iconWrapperLarge;
      }
    })();
    const removeButton = (() => {
      if(this.state.isSelected) {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return (
            <div style={iconWrapperStyle}
              onClick={this.removeEntry}>
              <img height={imageSize} width={imageSize}
                style={SingleSecurityInput.STYLE.iconClickableStyle}
                src={SingleSecurityInput.PATH + 'remove-purple.svg'}/>
            </div>);
        } else {
          return (
            <div style={iconWrapperStyle}
              onClick={this.removeEntry}>
              <img height={imageSize} width={imageSize}
                style={SingleSecurityInput.STYLE.iconClickableStyle}
                src={SingleSecurityInput.PATH + 'remove-purple.svg'}/>
              <div style={SingleSecurityInput.STYLE.iconLabel}>
                {SingleSecurityInput.REMOVE_TEXT}
              </div>
            </div>);
        }
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return (
            <div style={iconWrapperStyle}>
              <img height={imageSize} width={imageSize}
                src={SingleSecurityInput.PATH + 'remove-grey.svg'}/>
            </div>);
        } else {
          return (
            <div style={iconWrapperStyle}
              onClick={this.removeEntry}>
              <img height={imageSize} width={imageSize}
                src={SingleSecurityInput.PATH + 'remove-grey.svg'}/>
              <div style={SingleSecurityInput.STYLE.iconLabelReadonly}>
                {SingleSecurityInput.REMOVE_TEXT}
              </div>
            </div>);
        }
      }
    })();
    const confirmationButton = (() => {
      return (
        <Button label={SingleSecurityInput.SUBMIT_CHANGES_TEXT}
          onClick={this.onSubmitChange}/>);
    })();
    let displayValue = (() => {
      if(this.props.value !== null) {
        return this.props.value.symbol;
      } else {
        return '';
      }
    })();
    return (
      <div>
        <input
          style={SingleSecurityInput.STYLE.textBox}
          className={css(SingleSecurityInput.EXTRA_STYLE.effects)}
          value={displayValue}
          onFocus={this.onOpen}
          onClick={this.onOpen}/>
        <div style={visibility}>
          <Modal displaySize={this.props.displaySize}
            width='300px' height={SingleSecurityInput.MODAL_HEIGHT}
            onClose={this.onClose}>
            <div style={SingleSecurityInput.STYLE.modalPadding}>
              <div style={SingleSecurityInput.STYLE.header}>
                <div style={SingleSecurityInput.STYLE.headerText}>
                  {SingleSecurityInput.MODAL_HEADER}
                </div>
                <img src={SingleSecurityInput.PATH + 'close.svg'}
                  height='20px'
                  width='20px'
                  style={SingleSecurityInput.STYLE.clickable}
                  onClick={this.onClose}/>
              </div>
              {inputField}
              <SymbolBox
                value={this.state.localValue}
                displaySize={this.props.displaySize}
                isSelected={this.state.isSelected}
                onClick={this.selectEntry}/>
              <div style={iconRowStyle}>
                {removeButton}
              </div>
              <HLine color={'#e6e6e6'}/>
              <div style={SingleSecurityInput.STYLE.buttonWrapper}>
                {confirmationButton}
              </div>
            </div>
          </Modal>
        </div>
      </div>);
  }

  private addEntry(security: Nexus.Security) {
    this.setState({
      inputString: '',
      localValue: security
    });
  }

  private onInputChange(value: string) {
    this.setState({ inputString: value });
  }

  private onSubmitChange() {
    if(this.props.value !== this.state.localValue) {
      this.props.onChange(this.state.localValue);
    }
    this.onClose();
  }

  private removeEntry() {
    if(this.state.isSelected) {
      this.setState({
        isSelected: false,
        localValue: null
      });
    }
  }

  private selectEntry(bool: boolean) {
    this.setState({ isSelected: bool});
  }

  private onOpen() {
    this.setState({
      isEditing: true,
      isSelected: false,
      localValue: this.props.value
    });
  }

  private onClose() {
    this.setState({
      isEditing: false
    });
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
    modalPadding: {
      padding: '18px'
    },
    clickable: {
      cursor: 'pointer' as 'pointer'
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
      flexGrow: 1,
      cursor: 'default' as 'default'
    },
    symbolHeader: {
      boxSizing: 'border-box' as 'border-box',
      backgroundColor: '#FFFFFF',
      height: '40px',
      maxWidth: '264px',
      color: '#4B23A0',
      font: '500 14px Roboto',
      paddingLeft: '10px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      borderBottom: '1px solid #C8C8C8',
      position: 'sticky' as 'sticky',
      top: 0,
      cursor: 'default' as 'default'
    },
    symbolBoxWrapper: {
      height: '76px',
      border: '1px solid #C8C8C8',
    },
    symbol: {
      height: '34px',
      paddingLeft: '10px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center', 
    },
    iconClickableStyle: {
      cursor: 'pointer' as 'pointer'
    },
    iconWrapperSmall: {
      height: '24px',
      width: '24px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      pointer: 'pointer' as 'pointer'
    },
    iconWrapperSmallReadonly: {
      height: '24px',
      width: '24px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      pointer: 'default' as 'default'
    },
    iconWrapperLarge: {
      height: '16px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      pointer: 'pointer' as 'pointer'
    },
    iconWrapperLargeReadonly: {
      height: '16px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      pointer: 'default' as 'default'
    },
    iconLabel: {
      color: '#333333',
      font: '400 14px Roboto',
      paddingLeft: '8px',
      cursor: 'pointer' as 'pointer'
    },
    iconLabelReadonly: {
      color: '#C8C8C8',
      font: '400 14px Roboto',
      paddingLeft: '8px',
      cursor: 'default' as 'default'
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
    iconRowLarge: {
      height: '16px',
      width: '100%',
      marginBottom: '30px',
      marginTop: '30px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      justifyContent: 'space-evenly' as 'space-evenly'
    },
    buttonWrapper: {
      marginTop: '30px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'wrap' as 'wrap',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center'
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
      }
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
      cursor: 'pointer' as 'pointer',
      ':active': {
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
      ':hover': {
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
  private static readonly IMAGE_SIZE_SMALL_VIEWPORT = '20px';
  private static readonly IMAGE_SIZE_LARGE_VIEWPORT = '16px';
  private static readonly MODAL_HEADER = 'Edit Symbols';
  private static readonly MODAL_HEIGHT = '363px';
  private static readonly PATH =
    'resources/account_page/compliance_page/security_input/';
  private static readonly REMOVE_TEXT = 'Remove';
  private static readonly SUBMIT_CHANGES_TEXT = 'Submit Changes';
}

interface InputFieldProperties {

  /** The current value of the input field. */
  value: string;

  /** Called when the displayed value changes.
   * @param value - The new value.
   */
  onChange: (value: string) => void;

  /** Called when the value is submitted.
   * @param value - The compliance value that is being submitted.
   */
  onEnter: (value: Nexus.Security) => void;
}

/** The field that allows the user to add a new entry to the list. */
class InputField extends React.Component<InputFieldProperties> {
  constructor(props: InputFieldProperties) {
    super(props);
    this.onInputChange = this.onInputChange.bind(this);
    this.onKeyDown = this.onKeyDown.bind(this);
  }

  public render() {
    return (<input
      className={css(InputField.EXTRA_STYLE.effects)}
      style={InputField.STYLE.findSymbolBox}
      placeholder={InputField.PLACEHOLDER_TEXT}
      onChange={this.onInputChange}
      onKeyDown={this.onKeyDown}
      value={this.props.value}/>);
  }

  private onInputChange(event: React.ChangeEvent<HTMLInputElement>) {
    this.props.onChange(event.target.value);
  }

  private onKeyDown(event: React.KeyboardEvent<HTMLInputElement>) {
    if(event.keyCode === 13) {
      const newSecurity =
          new Nexus.Security(
            this.props.value,
            Nexus.MarketCode.NONE,
            Nexus.DefaultCountries.CA);
      this.props.onEnter(newSecurity);
    }
  }

  private static readonly STYLE = {
    findSymbolBox: {
      width: '100%',
      boxSizing: 'border-box' as 'border-box',
      font: '400 14px Roboto',
      height: '34px',
      paddingLeft: '10px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      marginBottom: '18px'
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
      }
    }
  });
  private static readonly PLACEHOLDER_TEXT = 'Find symbol here';
}


interface SymbolsBoxProperties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** The index of the currently selected value. */
  isSelected: boolean;

  /** The list of securities to display. */
  value: Nexus.Security;

  /** Called when a list item is clicked on.
   * @param index - The index of the selected security.
   */
  onClick?: (isSelected: boolean) => void;
}

/** A component that displays a list of symbols. */
class SymbolBox extends React.Component<SymbolsBoxProperties> {
  constructor(props: SymbolsBoxProperties) {
    super(props);
    this.onClick = this.onClick.bind(this);
  }
  
  public render() {
    const scrollHeader = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return (
          <div style={SymbolBox.STYLE.scrollBoxHeaderSmall}>
            {'Added Symbols'}
          </div>);
      } else {
        return (
          <div style={SymbolBox.STYLE.scrollBoxHeaderLarge}>
            {'Added Symbols'}
          </div>);
      }
    })();
    const selectedSecuritiesBox = (() => {
      const displaySize = this.props.displaySize;
      if(displaySize === DisplaySize.SMALL) {
        return SymbolBox.STYLE.scrollBoxSmall;
      } else {
        return SymbolBox.STYLE.scrollBoxBig;
      }
    })();
    const symbol = (() => {
      const displayValue = (() => {
        if(this.props.value !== null) {
          return this.props.value.symbol;
        }else {
          return '';
        }
      })();
      if(this.props.isSelected) {
        return (
          <div style={SymbolBox.STYLE.scrollBoxEntrySelected}
            onClick={this.onClick}>
            {displayValue}
          </div>);
      } else {
        return (
          <div style={SymbolBox.STYLE.scrollBoxEntry}
            onClick={this.onClick}>
            {displayValue}
          </div>);
      }
    })();
    return (
      <div style={selectedSecuritiesBox}>
        {scrollHeader}
        {symbol}
      </div>);
  }

  private onClick() {
    this.props.onClick(!this.props.isSelected);
  }

  private static readonly STYLE = {
    headerText: {
      font: '400 16px Roboto',
      flexGrow: 1,
      cursor: 'default' as 'default'
    },
    scrollBoxSmall: {
      boxSizing: 'border-box' as 'border-box',
      height: '76px',
      width: '246px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'auto' as 'auto'
    },
    scrollBoxBig: {
      boxSizing: 'border-box' as 'border-box',
      height: '76px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'auto' as 'auto'
    },
    scrollBoxHeaderSmall: {
      boxSizing: 'border-box' as 'border-box',
      backgroundColor: '#FFFFFF',
      height: '40px',
      maxWidth: '246px',
      color: '#4B23A0',
      font: '500 14px Roboto',
      paddingLeft: '10px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      borderBottom: '1px solid #C8C8C8',
      position: 'sticky' as 'sticky',
      top: 0,
      cursor: 'default' as 'default'
    },
    scrollBoxHeaderLarge: {
      boxSizing: 'border-box' as 'border-box',
      backgroundColor: '#FFFFFF',
      height: '40px',
      maxWidth: '264px',
      color: '#4B23A0',
      font: '500 14px Roboto',
      paddingLeft: '10px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      borderBottom: '1px solid #C8C8C8',
      position: 'sticky' as 'sticky',
      top: 0,
      cursor: 'default' as 'default'
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
      alignItems: 'center' as 'center',
      cursor: 'pointer' as 'pointer'
    },
    scrollBoxEntryReadonly: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      width: '100%',
      backgroundColor: '#FFFFFF',
      color: '#000000',
      font: '400 14px Roboto',
      paddingLeft: '10px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      cursor: 'default' as 'default'
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
      alignItems: 'center' as 'center',
      cursor: 'pointer' as 'pointer'
    }
  };
}
