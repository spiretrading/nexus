import { css, StyleSheet } from 'aphrodite/no-important';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Button, DisplaySize, HLine, Input, Modal, ScopeItemInput } from '..';

interface Properties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** Determines if the component is readOnly. */
  readOnly?: boolean;

  /** Determines if the component is disabled. */
  disabled?: boolean;

  /** The scope to display. */
  value?: Nexus.Scope;

  /** Called when the list of values changes.
   * @param value - The new list.
   */
  onChange?: (value: Nexus.Scope) => void;
}

type ScopeItem = Nexus.CountryCode | Nexus.Venue | Nexus.Ticker | Nexus.Scope;

interface State {
  inputString: string;
  isEditing: boolean;
  scopeItems: ScopeItem[];
  selection: number;
}

/** A component that displays and edits a scope. */
export class ScopeInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      inputString: '',
      isEditing: false,
      scopeItems: decomposeScope(this.props.value ?? new Nexus.Scope()),
      selection: -1
    };
  }

  public render() {
    const visibility = (() => {
      if(!this.state.isEditing) {
        return ScopeInput.STYLE.hidden;
      } else {
        return null;
      }
    })();
    const headerText = (() => {
      if(this.props.readOnly) {
        return ScopeInput.MODAL_HEADER_READONLY;
      } else {
        return ScopeInput.MODAL_HEADER;
      }
    })();
    const inputField = (() => {
      if(this.props.readOnly) {
        return null;
      } else {
        return (
          <ScopeItemInput
            style={{width: '100%', marginBottom: '18px'}}
            value={this.state.inputString}
            disabled={this.props.disabled}
            onChange={this.onInputChange}
            onEnter={this.addEntry}/>);
      }
    })();
    const iconRowStyle = (() => {
      if(this.props.readOnly) {
        return ScopeInput.STYLE.hidden;
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return ScopeInput.STYLE.iconRowSmall;
        } else {
          return ScopeInput.STYLE.iconRowBig;
        }
      }
    })();
    const imageSize = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ScopeInput.IMAGE_SIZE_SMALL_VIEWPORT;
      } else {
        return ScopeInput.IMAGE_SIZE_LARGE_VIEWPORT;
      }
    })();
    const iconWrapperStyle = (() => {
      const displaySize = this.props.displaySize;
      if(displaySize === DisplaySize.SMALL && this.props.readOnly) {
        return ScopeInput.STYLE.iconWrapperSmallReadonly;
      } else if(displaySize === DisplaySize.SMALL && !this.props.readOnly) {
        return ScopeInput.STYLE.iconWrapperSmall;
      } else if(displaySize === DisplaySize.LARGE && this.props.readOnly) {
        return ScopeInput.STYLE.iconWrapperLargeReadonly;
      } else {
        return ScopeInput.STYLE.iconWrapperLarge;
      }
    })();
    const uploadButton = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return (
          <div style={iconWrapperStyle}>
            <img height={imageSize} width={imageSize}
              src={ScopeInput.PATH + 'upload-grey.svg'}/>
          </div>);
      } else {
        return (
          <div style={iconWrapperStyle}>
            <img height={imageSize} width={imageSize}
              src={ScopeInput.PATH + 'upload-grey.svg'}/>
            <div style={ScopeInput.STYLE.iconLabelReadonly}>
              {ScopeInput.UPLOAD_TEXT}
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
                style={ScopeInput.STYLE.iconClickableStyle}
                src={ScopeInput.PATH + 'remove-purple.svg'}/>
            </div>);
        } else {
          return (
            <div style={iconWrapperStyle}
                onClick={this.removeEntry}>
              <img height={imageSize} width={imageSize}
                style={ScopeInput.STYLE.iconClickableStyle}
                src={ScopeInput.PATH + 'remove-purple.svg'}/>
              <div style={ScopeInput.STYLE.iconLabel}>
                {ScopeInput.REMOVE_TEXT}
              </div>
            </div>);
        }
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return (
            <div style={iconWrapperStyle}>
              <img height={imageSize} width={imageSize}
                src={ScopeInput.PATH + 'remove-grey.svg'}/>
            </div>);
        } else {
          return (
            <div style={iconWrapperStyle} onClick={this.removeEntry}>
              <img height={imageSize} width={imageSize}
                src={ScopeInput.PATH + 'remove-grey.svg'}/>
              <div style={ScopeInput.STYLE.iconLabelReadonly}>
                {ScopeInput.REMOVE_TEXT}
              </div>
            </div>);
        }
      }
    })();
    const confirmationButton = (() => {
      if(this.props.readOnly) {
        return (
          <Button label={ScopeInput.CONFIRM_TEXT}
            onClick={this.onClose}
            style={{width: '100%'}}/>);
      } else {
        return (
          <Button label={ScopeInput.SUBMIT_CHANGES_TEXT}
            onClick={this.onSubmitChange}
            style={{width: '100%'}}/>);
      }
    })();
    const displayText = makeDisplayText(this.props.value ?? new Nexus.Scope());
    return (
      <div>
        <Input
          readOnly
          disabled={this.props.disabled}
          className={!this.props.readOnly && !this.props.disabled ?
            css(ScopeInput.INPUT_STYLE.interactive) : undefined}
          style={ScopeInput.STYLE.textBox}
          value={displayText}
          onFocus={this.onOpen}
          onClick={this.onOpen}/>
        <div style={visibility}>
          <Modal
              title={headerText} onClose={this.onClose}>
            <div style={ScopeInput.STYLE.contentWrapper}>
              {inputField}
              <ScopeItemList
                displaySize={this.props.displaySize}
                readOnly={this.props.readOnly}
                value={this.state.scopeItems}
                selection={this.state.selection}
                onClick={this.selectEntry}/>
              <div style={iconRowStyle}>
                {removeButton}
                {uploadButton}
              </div>
              <HLine color={'#e6e6e6'}/>
              <div style={ScopeInput.STYLE.buttonWrapper}>
                {confirmationButton}
              </div>
            </div>
          </Modal>
        </div>
      </div>);
  }

  private addEntry = (parameter: Nexus.Scope) => {
    this.setState({
      inputString: '',
      scopeItems: this.state.scopeItems.concat(parameter)
    });
  }

  private onInputChange = (value: string) => {
    this.setState({inputString: value});
  }

  private onSubmitChange = () => {
    this.props.onChange?.(recomposeScope(this.state.scopeItems));
    this.onClose();
  }

  private removeEntry = () => {
    if(this.state.selection !== -1) {
      this.setState({
        selection: -1,
        scopeItems: this.state.scopeItems.slice(
          0, this.state.selection).concat(
            this.state.scopeItems.slice(this.state.selection + 1))
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
      scopeItems: decomposeScope(this.props.value ?? new Nexus.Scope())
    });
  }

  private onClose = () => {
    this.setState({
      isEditing: false
    });
  }

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
  private static readonly CONFIRM_TEXT = 'OK';
  private static readonly IMAGE_SIZE_SMALL_VIEWPORT = '20px';
  private static readonly IMAGE_SIZE_LARGE_VIEWPORT = '16px';
  private static readonly MODAL_HEADER = 'Edit Scope';
  private static readonly MODAL_HEADER_READONLY = 'Scope';
  private static readonly PATH =
    'resources/account_page/compliance_page/ticker_input/';
  private static readonly REMOVE_TEXT = 'Remove';
  private static readonly SUBMIT_CHANGES_TEXT = 'Submit Changes';
  private static readonly UPLOAD_TEXT = 'Upload';
}

interface ScopeItemListProperties {
  displaySize: DisplaySize;
  readOnly?: boolean;
  selection: number;
  value: ScopeItem[];
  onClick?: (index: number) => void;
}

class ScopeItemList extends React.Component<ScopeItemListProperties> {
  public render() {
    const scrollHeader = (() => {
      if(!this.props.readOnly) {
        return (
          <div style={ScopeItemList.STYLE.scrollBoxHeaderSmall}>
            Subscopes
          </div>);
      } else {
        return null;
      }
    })();
    const selectedScopesBox = (() => {
      const displaySize = this.props.displaySize;
      if(displaySize === DisplaySize.SMALL && this.props.readOnly) {
        return ScopeItemList.STYLE.scrollBoxSmallReadonly;
      } else if(displaySize === DisplaySize.SMALL && !this.props.readOnly) {
        return ScopeItemList.STYLE.scrollBoxSmall;
      } else if(displaySize !== DisplaySize.SMALL && this.props.readOnly) {
        return ScopeItemList.STYLE.scrollBoxBigReadonly;
      } else if (displaySize !== DisplaySize.SMALL && !this.props.readOnly) {
        return ScopeItemList.STYLE.scrollBoxBig;
      }
    })();
    const entries = [];
    for(let i = 0; i < this.props.value.length; ++i) {
      const symbol = makeScopeItemText(this.props.value[i]);
      if(this.props.readOnly) {
        entries.push(
          <div
              key={i}
              className={css(ScopeItemList.EXTRA_STYLE.entryReadonly)}>
            {symbol}
          </div>);
      } else if(this.props.selection === i) {
        entries.push(
          <div
              key={i}
              className={css(ScopeItemList.EXTRA_STYLE.entrySelected)}
              onClick={this.selectEntry.bind(this, i)}>
            {symbol}
          </div>);
      } else {
        entries.push(
          <div key={i} className={css(ScopeItemList.EXTRA_STYLE.entry)}
              onClick={this.selectEntry.bind(this, i)}>
            {symbol}
          </div>);
      }
    }
    return (
      <div style={selectedScopesBox}>
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

function isCountryCode(x: any): x is Nexus.CountryCode {
  return typeof x?.code === 'number';
}

function isTicker(x: any): x is Nexus.Ticker {
  return typeof x?.symbol === 'string';
}

function isVenue(x: any): x is Nexus.Venue {
  return !isTicker(x) && typeof x?.toString === 'string';
}

function isScope(x: any): x is Nexus.Scope {
  return typeof x?.isGlobal === 'boolean';
}

function decomposeScope(scope: Nexus.Scope): ScopeItem[] {
  if(scope.isGlobal) {
    return [scope];
  }
  const countries = [];
  for(const country of scope.countries) {
    countries.push(country);
  }
  countries.sort((a: Nexus.CountryCode, b: Nexus.CountryCode) => {
    const aCode = Nexus.defaultCountryDatabase.fromCode(a).twoLetterCode;
    const bCode = Nexus.defaultCountryDatabase.fromCode(b).twoLetterCode;
    return aCode.localeCompare(bCode);
  });
  const venues = [];
  for(const venue of scope.venues) {
    venues.push(venue);
  }
  venues.sort((a: Nexus.Venue, b: Nexus.Venue) => {
    const aName = Nexus.defaultVenueDatabase.fromVenue(a).displayName;
    const bName = Nexus.defaultVenueDatabase.fromVenue(b).displayName;
    return aName.localeCompare(bName);
  });
  const tickers = [];
  for(const ticker of scope.tickers) {
    tickers.push(ticker);
  }
  tickers.sort((a: Nexus.Ticker, b: Nexus.Ticker) => {
    return a.toString().localeCompare(b.toString());
  })
  return [].concat(countries).concat(venues).concat(tickers);
}

function recomposeScope(members: ScopeItem[]): Nexus.Scope {
  const scope = new Nexus.Scope();
  for(const member of members) {
    if(isScope(member)) {
      scope.add(member);
    } else {
      scope.add(new Nexus.Scope(member as any));
    }
  }
  return scope;
}

function makeScopeItemText(item: ScopeItem): string {
  if(isCountryCode(item)) {
    return Nexus.defaultCountryDatabase.fromCode(item).twoLetterCode;
  } else if(isVenue(item)) {
    return Nexus.defaultVenueDatabase.fromVenue(item).displayName;
  } else if(isTicker(item)) {
    return item.toString();
  } else {
    if(item.isGlobal) {
      return '*';
    }
    for(const country of item.countries) {
      return makeScopeItemText(country);
    }
    for(const venue of item.venues) {
      return makeScopeItemText(venue);
    }
    for(const ticker of item.tickers) {
      return makeScopeItemText(ticker);
    }
  }
  return '';
}

function makeDisplayText(scope: Nexus.Scope): string {
  if(scope.isGlobal) {
    return '*';
  }
  let text = '';
  const members = decomposeScope(scope);
  for(const member of members) {
    if(text !== '') {
      text += ", ";
    }
    text += makeScopeItemText(member);
  }
  return text;
}
