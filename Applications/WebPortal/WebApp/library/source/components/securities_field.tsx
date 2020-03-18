import { css, StyleSheet } from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Button, DisplaySize, HLine, Modal, SecurityInput } from '..';

interface Properties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** Determines if the component is readonly. */
  readonly?: boolean;

  /** The list of securities to display. */
  value?: Nexus.Security[];

  /** Called when the list of values changes.
   * @param value - The new list.
   */
  onChange?: (value: Nexus.Security[]) => void;
}

interface State {
  inputString: string;
  isEditing: boolean;
  localValue: Nexus.Security[];
  selection: number;
}

/** A component that displays a list of securities. */
export class SecuritiesField extends React.Component<Properties, State> {
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
      localValue: this.props.value.slice(),
      selection: -1
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
        return SecuritiesField.STYLE.hidden;
      } else {
        return null;
      }
    })();
    const modalHeight = (() => {
      if(this.props.readonly) {
        return SecuritiesField.MODAL_HEIGHT_READONLY;
      } else {
        return SecuritiesField.MODAL_HEIGHT;
      }
    })();
    const headerText = (() => {
      if(this.props.readonly) {
        return SecuritiesField.MODAL_HEADER_READONLY;
      } else {
        return SecuritiesField.MODAL_HEADER;
      }
    })();
    const inputField = (() => {
      if(this.props.readonly) {
        return null;
      } else {
        return(
          <SecurityInput
            value={this.state.inputString}
            onChange={this.onInputChange}
            onEnter={this.addEntry}/>);
      }
    })();
    const iconRowStyle = (() => {
      if(this.props.readonly) {
        return SecuritiesField.STYLE.hidden;
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return SecuritiesField.STYLE.iconRowSmall;
        } else {
          return SecuritiesField.STYLE.iconRowBig;
        }
      }
    })();
    const imageSize = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SecuritiesField.IMAGE_SIZE_SMALL_VIEWPORT;
      } else {
        return SecuritiesField.IMAGE_SIZE_LARGE_VIEWPORT;
      }
    })();
    const iconWrapperStyle = (() => {
      const displaySize = this.props.displaySize;
      if(displaySize=== DisplaySize.SMALL && this.props.readonly) {
        return SecuritiesField.STYLE.iconWrapperSmallReadonly;
      } else if(displaySize=== DisplaySize.SMALL && !this.props.readonly) {
        return SecuritiesField.STYLE.iconWrapperSmall;
      } else if(displaySize=== DisplaySize.LARGE && this.props.readonly) {
        return SecuritiesField.STYLE.iconWrapperLargeReadonly;
      } else {
        return SecuritiesField.STYLE.iconWrapperLarge;
      }
    })();
    const uploadButton = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return (
          <div style={iconWrapperStyle}>
            <img height={imageSize} width={imageSize}
              src={SecuritiesField.PATH + 'upload-grey.svg'}/>
          </div>);
      } else {
        return (
          <div style={iconWrapperStyle}>
            <img height={imageSize} width={imageSize}
              src={SecuritiesField.PATH + 'upload-grey.svg'}/>
            <div style={SecuritiesField.STYLE.iconLabelReadonly}>
              {SecuritiesField.UPLOAD_TEXT}
            </div>
          </div>);
      }
    })();
    const removeButton = (() => {
      if(this.state.selection !== -1) {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return (
            <div style={iconWrapperStyle}
                onClick={this.removeEntry}>
              <img height={imageSize} width={imageSize}
                style={SecuritiesField.STYLE.iconClickableStyle}
                src={SecuritiesField.PATH + 'remove-purple.svg'}/>
            </div>);
        } else {
          return (
            <div style={iconWrapperStyle}
                onClick={this.removeEntry}>
              <img height={imageSize} width={imageSize}
                style={SecuritiesField.STYLE.iconClickableStyle}
                src={SecuritiesField.PATH + 'remove-purple.svg'}/>
              <div style={SecuritiesField.STYLE.iconLabel}>
                {SecuritiesField.REMOVE_TEXT}
              </div>
            </div>);
        }
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return (
            <div style={iconWrapperStyle}>
              <img height={imageSize} width={imageSize}
                src={SecuritiesField.PATH + 'remove-grey.svg'}/>
            </div>);
        } else {
          return (
            <div style={iconWrapperStyle}
                onClick={this.removeEntry}>
              <img height={imageSize} width={imageSize}
                src={SecuritiesField.PATH + 'remove-grey.svg'}/>
              <div style={SecuritiesField.STYLE.iconLabelReadonly}>
                {SecuritiesField.REMOVE_TEXT}
              </div>
            </div>);
        }
      }
    })();
    const confirmationButton = (() => {
      if(this.props.readonly) {
        return (
          <Button label={SecuritiesField.CONFIRM_TEXT}
            onClick={this.onClose}/>);
      } else {
        return (
          <Button label={SecuritiesField.SUBMIT_CHANGES_TEXT}
            onClick={this.onSubmitChange}/>);
      }
    })();
    let displayValue  = '';
    for(let i = 0; i < this.props.value.length; ++i) {
      const symbol = this.props.value[i].symbol;
      displayValue = displayValue.concat(symbol);
      if(this.props.value.length > 1 && i < this.props.value.length - 1) {
        displayValue = displayValue.concat(', ');
      }
    }
    return(
      <div>
        <input
          style={SecuritiesField.STYLE.textBox}
          className={css(SecuritiesField.EXTRA_STYLE.effects)}
          value={displayValue}
          onFocus={this.onOpen}
          onClick={this.onOpen}/>
        <div style={visibility}>
          <Modal displaySize={this.props.displaySize} 
              width='300px' height={modalHeight}
              onClose={this.onClose}>
            <div style={SecuritiesField.STYLE.modalPadding}>
              <div style={SecuritiesField.STYLE.header}>
                <div style={SecuritiesField.STYLE.headerText}>
                  {headerText}
                </div>
                <img src={SecuritiesField.PATH + 'close.svg'}
                  height='20px'
                  width='20px'
                  style={SecuritiesField.STYLE.clickable}
                  onClick={this.onClose}/>
              </div>
              {inputField}
              <SymbolsBox 
                displaySize={this.props.displaySize}
                readonly={this.props.readonly}
                value={this.state.localValue}
                selection={this.state.selection}
                onClick={this.selectEntry}/>
              <div style={iconRowStyle}>
                {removeButton}
                {uploadButton}
              </div>
              <HLine color={'#e6e6e6'}/>
              <div style={SecuritiesField.STYLE.buttonWrapper}>
                {confirmationButton}
              </div>
            </div>
          </Modal>
        </div>
      </div>);
  }
  
  private addEntry(paramter: Nexus.Security) {
    this.setState({
      inputString: '',
      localValue: this.state.localValue.slice().concat(paramter)
    });
  }

  private onInputChange(value: string) {
    this.setState({inputString: value});
  }

  private onSubmitChange(){
    this.props.onChange(this.state.localValue);
    this.onClose();
  }

  private removeEntry() {
    if(this.state.selection !== -1) {
      this.setState({
        selection: -1,
        localValue: this.state.localValue.slice(0, this.state.selection).concat(
          this.state.localValue.slice(this.state.selection+1))
      });
    }
  }

  private selectEntry(index: number) {
    this.setState({selection: index});
  }

  private onOpen() {
    this.setState({
      inputString: '',
      isEditing: true,
      selection: -1,
      localValue: this.props.value.slice()
    });
  }

  private onClose(){
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
    iconWrapperLarge:  {
      height: '16px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      pointer: 'pointer' as 'pointer'
    },
    iconWrapperLargeReadonly:  {
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
    iconRowBig: {
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
  private static readonly CONFIRM_TEXT = 'OK';
  private static readonly IMAGE_SIZE_SMALL_VIEWPORT = '20px';
  private static readonly IMAGE_SIZE_LARGE_VIEWPORT = '16px';
  private static readonly MODAL_HEADER = 'Edit Symbols';
  private static readonly MODAL_HEADER_READONLY = 'Added Symbols';
  private static readonly MODAL_HEIGHT = '559px';
  private static readonly MODAL_HEIGHT_READONLY = '492px';
  private static readonly PATH =
    'resources/account_page/compliance_page/security_input/';
  private static readonly REMOVE_TEXT = 'Remove';
  private static readonly SUBMIT_CHANGES_TEXT = 'Submit Changes';
  private static readonly UPLOAD_TEXT = 'Upload';
}

interface SymbolsBoxProperties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** Determines if the component is readonly. */
  readonly?: boolean;

  /** The index of the currently selected value. */
  selection: number;

  /** The list of securities to display. */
  value: Nexus.Security[];

  /** Called when a list item is clicked on.
   * @param index - The index of the selected security.
   */
  onClick?: (index: number) => void;
}

/** A component that displays a list of symbols. */
class SymbolsBox extends React.Component<SymbolsBoxProperties> {
  public render() {
    const scrollHeader = (() => {
      if(!this.props.readonly) {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return(
            <div style={SymbolsBox.STYLE.scrollBoxHeaderSmall}>
              {'Added Symbols'}
            </div>);
        } else {
          return(
            <div style={SymbolsBox.STYLE.scrollBoxHeaderLarge}>
              {'Added Symbols'}
            </div>);
        }
      } else {
        return null;
      }
    })();
    const selectedSecuritiesBox = (() => {
      const displaySize = this.props.displaySize;
      if(displaySize === DisplaySize.SMALL && this.props.readonly) {
        return SymbolsBox.STYLE.scrollBoxSmallReadonly;
      } else if (displaySize === DisplaySize.SMALL && !this.props.readonly ) {
        return SymbolsBox.STYLE.scrollBoxSmall;
      } else if(displaySize !== DisplaySize.SMALL && this.props.readonly) {
        return SymbolsBox.STYLE.scrollBoxBigReadonly;
      } else if (displaySize !== DisplaySize.SMALL && !this.props.readonly) {
        return SymbolsBox.STYLE.scrollBoxBig;
      }
    })();
    const entries = [];
    for(let i = 0; i < this.props.value.length; ++i) {
        const symbol = this.props.value[i].symbol;
        if(this.props.readonly) {
          entries.push(
            <div style={SymbolsBox.STYLE.scrollBoxEntryReadonly}>
              {symbol}
            </div>);
        } else if(this.props.selection === i) {
          entries.push(
            <div style={SymbolsBox.STYLE.scrollBoxEntrySelected}
                onClick={this.selectEntry.bind(this, i)}>
              {symbol}
            </div>);
        } else {
          entries.push(
            <div style={SymbolsBox.STYLE.scrollBoxEntry}
                onClick={this.selectEntry.bind(this, i)}>
              {symbol}
            </div>);
        }
    }
    return (
      <div style={selectedSecuritiesBox}>
        {scrollHeader}
        {entries}
      </div>);
  }

  private selectEntry(index: number) {
    if(!this.props.readonly) {
      if(index === this.props.selection) {
        this.props.onClick(-1);
      } else {
        this.props.onClick(index);
      }
    }
  }

  private static readonly STYLE = {
    headerText: {
      font: '400 16px Roboto',
      flexGrow: 1,
      cursor: 'default' as 'default'
    },
    scrollBoxSmall: {
      boxSizing: 'border-box' as 'border-box',
      height: '246px',
      width: '246px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'auto' as 'auto'
    },
    scrollBoxSmallReadonly: {
      boxSizing: 'border-box' as 'border-box',
      height: '342px',
      width: '246px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'auto' as 'auto'
    },
    scrollBoxBig: {
      boxSizing: 'border-box' as 'border-box',
      height: '280px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'auto' as 'auto'
    },
    scrollBoxBigReadonly: {
      boxSizing: 'border-box' as 'border-box',
      height: '342px',
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
