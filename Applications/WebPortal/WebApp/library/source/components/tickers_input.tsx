import { css, StyleSheet } from 'aphrodite/no-important';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Button, DisplaySize, HLine, Input, Modal, TickerInput } from '..';

interface Properties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** Determines if the component is readOnly. */
  readOnly?: boolean;

  /** Determines if the component is disabled. */
  disabled?: boolean;

  /** The list of tickers to display. */
  value?: Nexus.Ticker[];

  /** Called when the list of values changes.
   * @param value - The new list.
   */
  onChange?: (value: Nexus.Ticker[]) => void;
}

interface State {
  inputString: string;
  isEditing: boolean;
  localValue: Nexus.Ticker[];
  selection: number;
}

/** A component that displays a list of tickers. */
export class TickersInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      inputString: '',
      isEditing: false,
      localValue: (this.props.value ?? []).slice(),
      selection: -1
    };
    this.fileInputRef = React.createRef<HTMLInputElement>();
  }

  public render() {
    const visibility = (() => {
      if(!this.state.isEditing) {
        return TickersInput.STYLE.hidden;
      } else {
        return null;
      }
    })();
    const headerText = (() => {
      if(this.props.readOnly) {
        return TickersInput.MODAL_HEADER_READONLY;
      } else {
        return TickersInput.MODAL_HEADER;
      }
    })();
    const inputField = (() => {
      if(this.props.readOnly) {
        return null;
      } else {
        return (
          <TickerInput
            style={{width: '100%', marginBottom: '18px'}}
            value={this.state.inputString}
            disabled={this.props.disabled}
            onChange={this.onInputChange}
            onEnter={this.addEntry}/>);
      }
    })();
    const iconRowStyle = (() => {
      if(this.props.readOnly) {
        return TickersInput.STYLE.hidden;
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return TickersInput.STYLE.iconRowSmall;
        } else {
          return TickersInput.STYLE.iconRowBig;
        }
      }
    })();
    const imageSize = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return TickersInput.IMAGE_SIZE_SMALL_VIEWPORT;
      } else {
        return TickersInput.IMAGE_SIZE_LARGE_VIEWPORT;
      }
    })();
    const iconWrapperStyle = (() => {
      const displaySize = this.props.displaySize;
      if(displaySize === DisplaySize.SMALL && this.props.readOnly) {
        return TickersInput.STYLE.iconWrapperSmallReadonly;
      } else if(displaySize === DisplaySize.SMALL && !this.props.readOnly) {
        return TickersInput.STYLE.iconWrapperSmall;
      } else if(displaySize === DisplaySize.LARGE && this.props.readOnly) {
        return TickersInput.STYLE.iconWrapperLargeReadonly;
      } else {
        return TickersInput.STYLE.iconWrapperLarge;
      }
    })();
    const uploadButton = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return (
          <div style={iconWrapperStyle} onClick={this.onUploadClick}>
            <img height={imageSize} width={imageSize}
              style={TickersInput.STYLE.iconClickableStyle}
              src={TickersInput.PATH + 'upload-purple.svg'}/>
          </div>);
      } else {
        return (
          <div style={iconWrapperStyle} onClick={this.onUploadClick}>
            <img height={imageSize} width={imageSize}
              style={TickersInput.STYLE.iconClickableStyle}
              src={TickersInput.PATH + 'upload-purple.svg'}/>
            <div style={TickersInput.STYLE.iconLabel}>
              {TickersInput.UPLOAD_TEXT}
            </div>
          </div>);
      }
    })();
    const removeButton = (() => {
      if(this.state.selection !== -1) {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return (
            <div style={iconWrapperStyle} onClick={this.removeEntry}>
              <img height={imageSize} width={imageSize}
                style={TickersInput.STYLE.iconClickableStyle}
                src={TickersInput.PATH + 'remove-purple.svg'}/>
            </div>);
        } else {
          return (
            <div style={iconWrapperStyle} onClick={this.removeEntry}>
              <img height={imageSize} width={imageSize}
                style={TickersInput.STYLE.iconClickableStyle}
                src={TickersInput.PATH + 'remove-purple.svg'}/>
              <div style={TickersInput.STYLE.iconLabel}>
                {TickersInput.REMOVE_TEXT}
              </div>
            </div>);
        }
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return (
            <div style={iconWrapperStyle}>
              <img height={imageSize} width={imageSize}
                src={TickersInput.PATH + 'remove-grey.svg'}/>
            </div>);
        } else {
          return (
            <div style={iconWrapperStyle} onClick={this.removeEntry}>
              <img height={imageSize} width={imageSize}
                src={TickersInput.PATH + 'remove-grey.svg'}/>
              <div style={TickersInput.STYLE.iconLabelReadonly}>
                {TickersInput.REMOVE_TEXT}
              </div>
            </div>);
        }
      }
    })();
    const confirmationButton = (() => {
      if(this.props.readOnly) {
        return (
          <Button label={TickersInput.CONFIRM_TEXT}
            onClick={this.onClose}
            style={{width: '100%'}}/>);
      } else {
        return (
          <Button label={TickersInput.SUBMIT_CHANGES_TEXT}
            onClick={this.onSubmitChange}
            style={{width: '100%'}}/>);
      }
    })();
    const value = this.props.value ?? [];
    let displayValue  = '';
    for(let i = 0; i < value.length; ++i) {
      const symbol = value[i].toString();
      displayValue = displayValue.concat(symbol);
      if(value.length > 1 && i < value.length - 1) {
        displayValue = displayValue.concat(', ');
      }
    }
    return (
      <div>
        <input type='file' accept='.txt,.csv' ref={this.fileInputRef}
          style={TickersInput.STYLE.hidden}
          onChange={this.onFileSelected}/>
        <Input
          readOnly
          disabled={this.props.disabled}
          className={!this.props.readOnly && !this.props.disabled ?
            css(TickersInput.INPUT_STYLE.interactive) : undefined}
          style={TickersInput.STYLE.textBox}
          value={displayValue}
          onFocus={this.onOpen}
          onClick={this.onOpen}/>
        <div style={visibility}>
          <Modal
              title={headerText} onClose={this.onClose}>
            <div style={TickersInput.STYLE.contentWrapper}>
              {inputField}
              <SymbolsField
                displaySize={this.props.displaySize}
                readOnly={this.props.readOnly}
                value={this.state.localValue}
                selection={this.state.selection}
                onClick={this.selectEntry}/>
              <div style={iconRowStyle}>
                {removeButton}
                {uploadButton}
              </div>
              <HLine color={'#e6e6e6'}/>
              <div style={TickersInput.STYLE.buttonWrapper}>
                {confirmationButton}
              </div>
            </div>
          </Modal>
        </div>
      </div>);
  }

  private readonly fileInputRef: React.RefObject<HTMLInputElement>;

  private onUploadClick = () => {
    this.fileInputRef.current?.click();
  }

  private onFileSelected = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if(!file) {
      return;
    }
    const reader = new FileReader();
    reader.onload = () => {
      const text = reader.result as string;
      const lines = text.split(/\r?\n/);
      const parsed: Nexus.Ticker[] = [];
      for(const line of lines) {
        const trimmed = line.trim();
        if(trimmed === '') {
          continue;
        }
        const ticker = Nexus.Ticker.parse(trimmed.toUpperCase());
        if(!ticker.equals(Nexus.Ticker.NONE)) {
          parsed.push(ticker);
        }
      }
      if(parsed.length > 0) {
        this.setState({
          localValue: this.state.localValue.concat(parsed)
        });
      }
    };
    reader.readAsText(file);
    event.target.value = '';
  }

  private addEntry = (parameter: Nexus.Ticker) => {
    this.setState({
      inputString: '',
      localValue: this.state.localValue.slice().concat(parameter)
    });
  }

  private onInputChange = (value: string) => {
    this.setState({inputString: value});
  }

  private onSubmitChange = () => {
    this.props.onChange?.(this.state.localValue);
    this.onClose();
  }

  private removeEntry = () => {
    if(this.state.selection !== -1) {
      this.setState({
        selection: -1,
        localValue: this.state.localValue.slice(0, this.state.selection).concat(
          this.state.localValue.slice(this.state.selection+1))
      });
    }
  }

  private selectEntry = (index: number) => {
    this.setState({selection: index});
  }

  private onOpen = () => {
    this.setState({
      inputString: '',
      isEditing: true,
      selection: -1,
      localValue: (this.props.value ?? []).slice()
    });
  }

  private onClose = () => {
    this.setState({
      isEditing: false
    });
  }

  private static readonly STYLE: Record<string, React.CSSProperties> = {
    textBox: {
      textOverflow: 'ellipsis',
      width: '100%',
      cursor: 'pointer'
    },
    hidden: {
      visibility: 'hidden',
      display: 'none'
    },
    contentWrapper: {
      boxSizing: 'border-box',
      width: '300px',
      padding: '30px 18px 18px 18px'
    },
    iconClickableStyle: {
      cursor: 'pointer'
    },
    iconWrapperSmall: {
      height: '24px',
      width: '24px',
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      justifyContent: 'center',
      cursor: 'pointer'
    },
    iconWrapperSmallReadonly: {
      height: '24px',
      width: '24px',
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      justifyContent: 'center',
      cursor: 'default'
    },
    iconWrapperLarge:  {
      height: '16px',
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      justifyContent: 'center',
      cursor: 'pointer'
    },
    iconWrapperLargeReadonly:  {
      height: '16px',
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      justifyContent: 'center',
      cursor: 'default'
    },
    iconLabel: {
      color: '#333333',
      font: '400 14px Roboto',
      paddingLeft: '8px',
      cursor: 'pointer'
    },
    iconLabelReadonly: {
      color: '#C8C8C8',
      font: '400 14px Roboto',
      paddingLeft: '8px',
      cursor: 'default'
    },
    iconRowSmall: {
      height: '24px',
      width: '100%',
      marginBottom: '30px',
      marginTop: '30px',
      display: 'flex',
      flexDirection: 'row',
      justifyContent: 'space-evenly'
    },
    iconRowBig: {
      height: '16px',
      width: '100%',
      marginBottom: '30px',
      marginTop: '30px',
      display: 'flex',
      flexDirection: 'row',
      justifyContent: 'space-evenly'
    },
    buttonWrapper: {
      marginTop: '30px',
      display: 'flex',
      flexDirection: 'row',
      flexWrap: 'wrap',
      justifyContent: 'center',
      alignItems: 'center'
    }
  };
  private static readonly INPUT_STYLE = StyleSheet.create({
    interactive: {
      borderColor: '#C8C8C8 !important',
      ':hover': {
        borderColor: '#684BC7 !important'
      },
      ':focus': {
        borderColor: '#684BC7 !important'
      }
    }
  });
  private static readonly CONFIRM_TEXT = 'OK';
  private static readonly IMAGE_SIZE_SMALL_VIEWPORT = '20px';
  private static readonly IMAGE_SIZE_LARGE_VIEWPORT = '16px';
  private static readonly MODAL_HEADER = 'Edit Symbols';
  private static readonly MODAL_HEADER_READONLY = 'Added Symbols';
  private static readonly PATH =
    'resources/account_page/compliance_page/ticker_input/';
  private static readonly REMOVE_TEXT = 'Remove';
  private static readonly SUBMIT_CHANGES_TEXT = 'Submit Changes';
  private static readonly UPLOAD_TEXT = 'Upload';
}

interface SymbolsFieldProperties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** Determines if the component is readOnly. */
  readOnly?: boolean;

  /** The index of the currently selected value. */
  selection: number;

  /** The list of tickers to display. */
  value: Nexus.Ticker[];

  /** Called when a list item is clicked on.
   * @param index - The index of the selected ticker.
   */
  onClick?: (index: number) => void;
}

/** A component that displays a list of symbols. */
class SymbolsField extends React.Component<SymbolsFieldProperties> {
  public render() {
    const scrollHeader = (() => {
      if(!this.props.readOnly) {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return (
            <div style={SymbolsField.STYLE.scrollBoxHeaderSmall}>
              Added Symbols
            </div>);
        } else {
          return (
            <div style={SymbolsField.STYLE.scrollBoxHeaderLarge}>
              Added Symbols
            </div>);
        }
      } else {
        return null;
      }
    })();
    const selectedTickersBox = (() => {
      const displaySize = this.props.displaySize;
      if(displaySize === DisplaySize.SMALL && this.props.readOnly) {
        return SymbolsField.STYLE.scrollBoxSmallReadonly;
      } else if(displaySize === DisplaySize.SMALL && !this.props.readOnly) {
        return SymbolsField.STYLE.scrollBoxSmall;
      } else if(displaySize !== DisplaySize.SMALL && this.props.readOnly) {
        return SymbolsField.STYLE.scrollBoxBigReadonly;
      } else if (displaySize !== DisplaySize.SMALL && !this.props.readOnly) {
        return SymbolsField.STYLE.scrollBoxBig;
      }
    })();
    const entries = [];
    for(let i = 0; i < this.props.value.length; ++i) {
      const symbol = this.props.value[i].toString();
      if(this.props.readOnly) {
        entries.push(
          <div key={i} className={css(SymbolsField.EXTRA_STYLE.entryReadonly)}>
            {symbol}
          </div>);
      } else if(this.props.selection === i) {
        entries.push(
          <div key={i} className={css(SymbolsField.EXTRA_STYLE.entrySelected)}
              onClick={this.selectEntry.bind(this, i)}>
            {symbol}
          </div>);
      } else {
        entries.push(
          <div key={i} className={css(SymbolsField.EXTRA_STYLE.entry)}
              onClick={this.selectEntry.bind(this, i)}>
            {symbol}
          </div>);
      }
    }
    return (
      <div style={selectedTickersBox}>
        {scrollHeader}
        {entries}
      </div>);
  }

  private selectEntry(index: number) {
    if(!this.props.readOnly) {
      if(index === this.props.selection) {
        this.props.onClick?.(-1);
      } else {
        this.props.onClick?.(index);
      }
    }
  }

  private static readonly STYLE: Record<string, React.CSSProperties> = {
    headerText: {
      font: '400 16px Roboto',
      flexGrow: 1,
      cursor: 'default'
    },
    scrollBoxSmall: {
      boxSizing: 'border-box',
      height: '246px',
      width: '246px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'auto'
    },
    scrollBoxSmallReadonly: {
      boxSizing: 'border-box',
      height: '342px',
      width: '246px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'auto'
    },
    scrollBoxBig: {
      boxSizing: 'border-box',
      height: '280px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'auto'
    },
    scrollBoxBigReadonly: {
      boxSizing: 'border-box',
      height: '342px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'auto'
    },
    scrollBoxHeaderSmall: {
      boxSizing: 'border-box',
      backgroundColor: '#FFFFFF',
      height: '40px',
      maxWidth: '246px',
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
    },
    scrollBoxHeaderLarge: {
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
    }
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    entry: {
      boxSizing: 'border-box',
      height: '34px',
      width: '100%',
      backgroundColor: '#FFFFFF',
      color: '#000000',
      font: '400 14px Roboto',
      paddingLeft: '10px',
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      cursor: 'pointer',
      ':hover': {
        backgroundColor: '#F8F8F8',
      }
    },
    entryReadonly: {
      boxSizing: 'border-box',
      height: '34px',
      width: '100%',
      backgroundColor: '#FFFFFF',
      color: '#000000',
      font: '400 14px Roboto',
      paddingLeft: '10px',
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      cursor: 'default'
    },
    entrySelected: {
      boxSizing: 'border-box',
      height: '34px',
      width: '100%',
      backgroundColor: '#684BC7',
      color: '#FFFFFF',
      font: '400 14px Roboto',
      paddingLeft: '10px',
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      cursor: 'pointer'
    }
  });
}
