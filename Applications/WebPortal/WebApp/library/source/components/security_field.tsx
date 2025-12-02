import { css, StyleSheet } from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Button, DisplaySize, HLine, Modal, SecurityInput } from '..';

interface Properties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** Determines if the component is readonly. */
  readonly?: boolean;

  /** The security to display. */
  value?: Nexus.Security;

  /**
   * Called when the value changes.
   * @param value - The new security.
   */
  onChange?: (value: Nexus.Security) => void;
}

interface State {
  inputString: string;
  isEditing: boolean;
  localValue: Nexus.Security;
  isSelected: boolean;
}

/** A component that displays a single security. */
export class SecurityField extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    value: new Nexus.Security('', Nexus.Venue.NONE),
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
    };
  }

  public render() {
    const visibility = (() => {
      if(!this.state.isEditing) {
        return SecurityField.STYLE.hidden;
      } else {
        return null;
      }
    })();
    const iconRowStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SecurityField.STYLE.iconRowSmall;
      } else {
        return SecurityField.STYLE.iconRowLarge;
      }
    })();
    const imageSize = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SecurityField.IMAGE_SIZE_SMALL_VIEWPORT;
      } else {
        return SecurityField.IMAGE_SIZE_LARGE_VIEWPORT;
      }
    })();
    const iconWrapperStyle = (() => {
      if (this.props.displaySize === DisplaySize.SMALL) {
        return SecurityField.STYLE.iconWrapperSmall;
      } else {
        return SecurityField.STYLE.iconWrapperLarge;
      }
    })();
    const removeButton = (() => {
      if(this.state.isSelected) {
        return (
          <div style={iconWrapperStyle}
              onClick={this.removeEntry}>
            <img height={imageSize} width={imageSize}
              style={SecurityField.STYLE.clickable}
              src={SecurityField.PATH + 'remove-purple.svg'}/>
            <div style={SecurityField.STYLE.iconLabel}>
              {SecurityField.REMOVE_TEXT}
            </div>
          </div>);
      } else {
        return (
          <div style={iconWrapperStyle}
              onClick={this.removeEntry}>
            <img height={imageSize} width={imageSize}
              src={SecurityField.PATH + 'remove-grey.svg'}/>
            <div style={SecurityField.STYLE.iconLabelReadonly}>
              {SecurityField.REMOVE_TEXT}
            </div>
          </div>);
      }
    })();
    const displayValue = (() => {
      if(this.props.value !== null) {
        return this.props.value.toString();
      } else {
        return '';
      }
    })();
    return (
      <div>
        <input
          style={SecurityField.STYLE.textBox}
          className={css(SecurityField.EXTRA_STYLE.effects)}
          value={displayValue}
          onFocus={this.onOpen}
          onClick={this.onOpen}/>
        <div style={visibility}>
          <Modal displaySize={this.props.displaySize}
              width='300px' height={SecurityField.MODAL_HEIGHT}
              onClose={this.onClose}>
            <div style={SecurityField.STYLE.modalPadding}>
              <div style={SecurityField.STYLE.header}>
                <div style={SecurityField.STYLE.headerText}>
                  {SecurityField.MODAL_HEADER}
                </div>
                <img src={SecurityField.PATH + 'close.svg'}
                  height='20px'
                  width='20px'
                  style={SecurityField.STYLE.clickable}
                  onClick={this.onClose}/>
              </div>
              <SecurityInput
                value={this.state.inputString}
                onChange={this.onInputChange}
                onEnter={this.addEntry}/>
              <SymbolField
                value={this.state.localValue}
                displaySize={this.props.displaySize}
                isSelected={this.state.isSelected}
                onClick={this.selectEntry}/>
              <div style={iconRowStyle}>
                {removeButton}
              </div>
              <HLine color={'#e6e6e6'}/>
              <div style={SecurityField.STYLE.buttonWrapper}>
                <Button label={SecurityField.SUBMIT_CHANGES_TEXT}
                  onClick={this.onSubmitChange}/>
              </div>
            </div>
          </Modal>
        </div>
      </div>);
  }

  private addEntry = (security: Nexus.Security) => {
    this.setState({
      inputString: '',
      localValue: security
    });
  }

  private onInputChange = (value: string) => {
    this.setState({ inputString: value });
  }

  private onSubmitChange = () => {
    if(this.props.value !== this.state.localValue) {
      this.props.onChange(this.state.localValue);
    }
    this.onClose();
  }

  private removeEntry = () => {
    if(this.state.isSelected) {
      this.setState({
        isSelected: false,
        localValue: null
      });
    }
  }

  private selectEntry = (isSelected: boolean) => {
    this.setState({ isSelected: isSelected});
  }

  private onOpen = () => {
    this.setState({
      inputString: '',
      isEditing: true,
      isSelected: false,
      localValue: this.props.value
    });
  }

  private onClose = () => {
    this.setState({
      isEditing: false
    });
  }

  private static readonly STYLE = {
    textBox: {
      textOverflow: 'ellipsis',
      boxSizing: 'border-box',
      height: '34px',
      display: 'flex',
      flexDirection: 'row',
      flexWrap: 'nowrap',
      alignItems: 'center',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      font: '400 14px Roboto',
      color: '#333333',
      flexGrow: 1,
      width: '100%',
      paddingLeft: '10px',
      cursor: 'pointer'
    } as React.CSSProperties,
    hidden: {
      visibility: 'hidden',
      display: 'none'
    } as React.CSSProperties,
    modalPadding: {
      padding: '18px'
    } as React.CSSProperties,
    clickable: {
      cursor: 'pointer'
    } as React.CSSProperties,
    header: {
      boxSizing: 'border-box',
      width: '100%',
      display: 'flex',
      flexDirection: 'row',
      justifyContent: 'space-between',
      height: '20px',
      marginBottom: '30px'
    } as React.CSSProperties,
    headerText: {
      font: '400 16px Roboto',
      flexGrow: 1,
      cursor: 'default'
    } as React.CSSProperties,
    symbolHeader: {
      boxSizing: 'border-box',
      backgroundColor: '#FFFFFF',
      height: '40px',
      maxWidth: '264px',
      color: '#4B23A0',
      font: '500 14px Roboto',
      paddingLeft: '10px',
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      borderBottom: '1px solid #C8C8C8',
      position: 'sticky',
      top: 0,
      cursor: 'default'
    } as React.CSSProperties,
    symbolBoxWrapper: {
      height: '76px',
      border: '1px solid #C8C8C8',
    } as React.CSSProperties,
    symbol: {
      height: '34px',
      paddingLeft: '10px',
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center'
    } as React.CSSProperties,
    iconWrapperSmall: {
      height: '24px',
      width: '24px',
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      justifyContent: 'center',
      pointer: 'pointer'
    } as React.CSSProperties,
    iconWrapperSmallReadonly: {
      height: '24px',
      width: '24px',
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      justifyContent: 'center',
      pointer: 'default'
    } as React.CSSProperties,
    iconWrapperLarge: {
      height: '16px',
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      justifyContent: 'center',
      pointer: 'pointer'
    } as React.CSSProperties,
    iconWrapperLargeReadonly: {
      height: '16px',
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      justifyContent: 'center',
      pointer: 'default'
    } as React.CSSProperties,
    iconLabel: {
      color: '#333333',
      font: '400 14px Roboto',
      paddingLeft: '8px',
      cursor: 'pointer'
    } as React.CSSProperties,
    iconLabelReadonly: {
      color: '#C8C8C8',
      font: '400 14px Roboto',
      paddingLeft: '8px',
      cursor: 'default'
    } as React.CSSProperties,
    iconRowSmall: {
      height: '24px',
      width: '100%',
      marginBottom: '30px',
      marginTop: '30px',
      display: 'flex',
      flexDirection: 'row',
      justifyContent: 'space-evenly'
    } as React.CSSProperties,
    iconRowLarge: {
      height: '16px',
      width: '100%',
      marginBottom: '30px',
      marginTop: '30px',
      display: 'flex',
      flexDirection: 'row',
      justifyContent: 'space-evenly'
    } as React.CSSProperties,
    buttonWrapper: {
      marginTop: '30px',
      display: 'flex',
      flexDirection: 'row',
      flexWrap: 'wrap',
      justifyContent: 'center',
      alignItems: 'center'
    } as React.CSSProperties
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

interface SymbolFieldProperties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** Determines if the security is selected or not. */
  isSelected: boolean;

  /** The security to display. */
  value: Nexus.Security;

  /** Called when security is clicked on.
   * @param isSelected - The boolean that says if it should be selected or not.
   */
  onClick?: (isSelected: boolean) => void;
}

/** A component that displays a list of symbols. */
class SymbolField extends React.Component<SymbolFieldProperties> {
  constructor(props: SymbolFieldProperties) {
    super(props);
  }
  
  public render() {
    const displayValue = (() => {
      if(this.props.value !== null) {
        return this.props.value.symbol;
      } else {
        return '';
      }
    })();
    const entryStyle = (() => {
      const isValidSecurity = this.props.value !== null &&
        this.props.value.symbol !== '';
      if(this.props.isSelected && isValidSecurity) {
        return SymbolField.EXTRA_STYLE.scrollBoxEntrySelected;
      } else if(!this.props.isSelected && isValidSecurity) {
        return SymbolField.EXTRA_STYLE.scrollBoxEntry;
      } else {
        return null;
      }
    })();
    return (
      <div style={SymbolField.STYLE.scrollBoxSmall}>
        <div style={SymbolField.STYLE.scrollBoxHeader}>
          'Added Symbol'
        </div>
        <div className={css(entryStyle)}
            onClick={this.onClick}>
          {displayValue}
        </div>
      </div>);
  }

  private onClick = () => {
    if(this.props.value.symbol !== ''){
      this.props.onClick(!this.props.isSelected);
    }
  }

  private static readonly STYLE = {
    scrollBoxSmall: {
      boxSizing: 'border-box',
      height: '76px',
      width: '100%',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'auto'
    } as React.CSSProperties,
    scrollBoxHeader: {
      boxSizing: 'border-box',
      backgroundColor: '#FFFFFF',
      height: '40px',
      maxWidth: '264px',
      color: '#4B23A0',
      font: '500 14px Roboto',
      paddingLeft: '10px',
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      borderBottom: '1px solid #C8C8C8',
      position: 'sticky',
      top: 0,
      cursor: 'default'
    } as React.CSSProperties
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
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
      cursor: 'pointer' as 'pointer',
      ':hover': {
        backgroundColor: '#F8F8F8',
      }
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
  });
}
