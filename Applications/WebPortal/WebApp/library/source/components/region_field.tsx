import { css, StyleSheet } from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Button, DisplaySize, HLine, Modal, RegionItemInput } from '..';

interface Properties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** Determines if the component is readonly. */
  readonly?: boolean;

  /** The region to display. */
  value?: Nexus.Region;

  /** Called when the list of values changes.
   * @param value - The new list.
   */
  onChange?: (value: Nexus.Region) => void;
}

type RegionItem = Nexus.CountryCode | Nexus.Venue | Nexus.Security | Nexus.Region;

interface State {
  inputString: string;
  isEditing: boolean;
  regionItems: RegionItem[];
  selection: number;
}

/** A component that displays and edits a region. */
export class RegionField extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    value: new Nexus.Region(),
    readonly: false,
    onChange: () => {}
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      inputString: '',
      isEditing: false,
      regionItems: decomposeRegion(this.props.value),
      selection: -1
    };
  }

  public render() {
    const visibility = (() => {
      if(!this.state.isEditing) {
        return RegionField.STYLE.hidden;
      } else {
        return null;
      }
    })();
    const modalHeight = (() => {
      if(this.props.readonly) {
        return RegionField.MODAL_HEIGHT_READONLY;
      } else {
        return RegionField.MODAL_HEIGHT;
      }
    })();
    const headerText = (() => {
      if(this.props.readonly) {
        return RegionField.MODAL_HEADER_READONLY;
      } else {
        return RegionField.MODAL_HEADER;
      }
    })();
    const inputField = (() => {
      if(this.props.readonly) {
        return null;
      } else {
        return (
          <RegionItemInput
            value={this.state.inputString}
            onChange={this.onInputChange}
            onEnter={this.addEntry}/>);
      }
    })();
    const iconRowStyle = (() => {
      if(this.props.readonly) {
        return RegionField.STYLE.hidden;
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return RegionField.STYLE.iconRowSmall;
        } else {
          return RegionField.STYLE.iconRowBig;
        }
      }
    })();
    const imageSize = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return RegionField.IMAGE_SIZE_SMALL_VIEWPORT;
      } else {
        return RegionField.IMAGE_SIZE_LARGE_VIEWPORT;
      }
    })();
    const iconWrapperStyle = (() => {
      const displaySize = this.props.displaySize;
      if(displaySize === DisplaySize.SMALL && this.props.readonly) {
        return RegionField.STYLE.iconWrapperSmallReadonly;
      } else if(displaySize === DisplaySize.SMALL && !this.props.readonly) {
        return RegionField.STYLE.iconWrapperSmall;
      } else if(displaySize === DisplaySize.LARGE && this.props.readonly) {
        return RegionField.STYLE.iconWrapperLargeReadonly;
      } else {
        return RegionField.STYLE.iconWrapperLarge;
      }
    })();
    const uploadButton = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return (
          <div style={iconWrapperStyle}>
            <img height={imageSize} width={imageSize}
              src={RegionField.PATH + 'upload-grey.svg'}/>
          </div>);
      } else {
        return (
          <div style={iconWrapperStyle}>
            <img height={imageSize} width={imageSize}
              src={RegionField.PATH + 'upload-grey.svg'}/>
            <div style={RegionField.STYLE.iconLabelReadonly}>
              {RegionField.UPLOAD_TEXT}
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
                style={RegionField.STYLE.iconClickableStyle}
                src={RegionField.PATH + 'remove-purple.svg'}/>
            </div>);
        } else {
          return (
            <div style={iconWrapperStyle}
                onClick={this.removeEntry}>
              <img height={imageSize} width={imageSize}
                style={RegionField.STYLE.iconClickableStyle}
                src={RegionField.PATH + 'remove-purple.svg'}/>
              <div style={RegionField.STYLE.iconLabel}>
                {RegionField.REMOVE_TEXT}
              </div>
            </div>);
        }
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return (
            <div style={iconWrapperStyle}>
              <img height={imageSize} width={imageSize}
                src={RegionField.PATH + 'remove-grey.svg'}/>
            </div>);
        } else {
          return (
            <div style={iconWrapperStyle} onClick={this.removeEntry}>
              <img height={imageSize} width={imageSize}
                src={RegionField.PATH + 'remove-grey.svg'}/>
              <div style={RegionField.STYLE.iconLabelReadonly}>
                {RegionField.REMOVE_TEXT}
              </div>
            </div>);
        }
      }
    })();
    const confirmationButton = (() => {
      if(this.props.readonly) {
        return (
          <Button label={RegionField.CONFIRM_TEXT}
            onClick={this.onClose}/>);
      } else {
        return (
          <Button label={RegionField.SUBMIT_CHANGES_TEXT}
            onClick={this.onSubmitChange}/>);
      }
    })();
    const displayText = makeDisplayText(this.props.value);
    return (
      <div>
        <input
          readOnly
          style={RegionField.STYLE.textBox}
          className={css(RegionField.EXTRA_STYLE.effects)}
          value={displayText}
          onFocus={this.onOpen}
          onClick={this.onOpen}/>
        <div style={visibility}>
          <Modal displaySize={this.props.displaySize} width='300px'
              height={modalHeight} onClose={this.onClose}>
            <div style={RegionField.STYLE.modalPadding}>
              <div style={RegionField.STYLE.header}>
                <div style={RegionField.STYLE.headerText}>
                  {headerText}
                </div>
                <img src={RegionField.PATH + 'close.svg'}
                  height='20px'
                  width='20px'
                  style={RegionField.STYLE.clickable}
                  onClick={this.onClose}/>
              </div>
              {inputField}
              <RegionItemList
                displaySize={this.props.displaySize}
                readonly={this.props.readonly}
                value={this.state.regionItems}
                selection={this.state.selection}
                onClick={this.selectEntry}/>
              <div style={iconRowStyle}>
                {removeButton}
                {uploadButton}
              </div>
              <HLine color={'#e6e6e6'}/>
              <div style={RegionField.STYLE.buttonWrapper}>
                {confirmationButton}
              </div>
            </div>
          </Modal>
        </div>
      </div>);
  }

  private addEntry = (parameter: Nexus.Region) => {
    this.setState({
      inputString: '',
      regionItems: this.state.regionItems.concat(parameter)
    });
  }

  private onInputChange = (value: string) => {
    this.setState({inputString: value});
  }

  private onSubmitChange = () => {
    this.props.onChange(recomposeRegion(this.state.regionItems));
    this.onClose();
  }

  private removeEntry = () => {
    if(this.state.selection !== -1) {
      this.setState({
        selection: -1,
        regionItems: this.state.regionItems.slice(
          0, this.state.selection).concat(
            this.state.regionItems.slice(this.state.selection + 1))
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
      regionItems: decomposeRegion(this.props.value)
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
    iconClickableStyle: {
      cursor: 'pointer'
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
    iconWrapperLarge:  {
      height: '16px',
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      justifyContent: 'center',
      pointer: 'pointer'
    } as React.CSSProperties,
    iconWrapperLargeReadonly:  {
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
    iconRowBig: {
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
  private static readonly MODAL_HEADER = 'Edit Region';
  private static readonly MODAL_HEADER_READONLY = 'Region';
  private static readonly MODAL_HEIGHT = '559px';
  private static readonly MODAL_HEIGHT_READONLY = '492px';
  private static readonly PATH =
    'resources/account_page/compliance_page/security_input/';
  private static readonly REMOVE_TEXT = 'Remove';
  private static readonly SUBMIT_CHANGES_TEXT = 'Submit Changes';
  private static readonly UPLOAD_TEXT = 'Upload';
}

interface RegionItemListProperties {
  displaySize: DisplaySize;
  readonly?: boolean;
  selection: number;
  value: RegionItem[];
  onClick?: (index: number) => void;
}

class RegionItemList extends React.Component<RegionItemListProperties> {
  public render() {
    const scrollHeader = (() => {
      if(!this.props.readonly) {
        return (
          <div style={RegionItemList.STYLE.scrollBoxHeaderSmall}>
            Subregions
          </div>);
      } else {
        return null;
      }
    })();
    const selectedRegionsBox = (() => {
      const displaySize = this.props.displaySize;
      if(displaySize === DisplaySize.SMALL && this.props.readonly) {
        return RegionItemList.STYLE.scrollBoxSmallReadonly;
      } else if(displaySize === DisplaySize.SMALL && !this.props.readonly) {
        return RegionItemList.STYLE.scrollBoxSmall;
      } else if(displaySize !== DisplaySize.SMALL && this.props.readonly) {
        return RegionItemList.STYLE.scrollBoxBigReadonly;
      } else if (displaySize !== DisplaySize.SMALL && !this.props.readonly) {
        return RegionItemList.STYLE.scrollBoxBig;
      }
    })();
    const entries = [];
    for(let i = 0; i < this.props.value.length; ++i) {
      const symbol = makeRegionItemText(this.props.value[i]);
      if(this.props.readonly) {
        entries.push(
          <div
              key={i}
              className={css(RegionItemList.EXTRA_STYLE.entryReadonly)}>
            {symbol}
          </div>);
      } else if(this.props.selection === i) {
        entries.push(
          <div
              key={i}
              className={css(RegionItemList.EXTRA_STYLE.entrySelected)}
              onClick={this.selectEntry.bind(this, i)}>
            {symbol}
          </div>);
      } else {
        entries.push(
          <div key={i} className={css(RegionItemList.EXTRA_STYLE.entry)}
              onClick={this.selectEntry.bind(this, i)}>
            {symbol}
          </div>);
      }
    }
    return (
      <div style={selectedRegionsBox}>
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
      cursor: 'default'
    } as React.CSSProperties,
    scrollBoxSmall: {
      boxSizing: 'border-box',
      height: '246px',
      width: '246px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'auto'
    } as React.CSSProperties,
    scrollBoxSmallReadonly: {
      boxSizing: 'border-box',
      height: '342px',
      width: '246px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'auto'
    } as React.CSSProperties,
    scrollBoxBig: {
      boxSizing: 'border-box',
      height: '280px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'auto'
    } as React.CSSProperties,
    scrollBoxBigReadonly: {
      boxSizing: 'border-box',
      height: '342px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'auto'
    } as React.CSSProperties,
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
    } as React.CSSProperties,
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
    } as React.CSSProperties
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    entry: {
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
    entryReadonly: {
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
    entrySelected: {
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

function isCountryCode(x: any): x is Nexus.CountryCode {
  return typeof x?.code === 'number';
}

function isSecurity(x: any): x is Nexus.Security {
  return typeof x?.symbol === 'string';
}

function isVenue(x: any): x is Nexus.Venue {
  return !isSecurity(x) && typeof x?.toString === 'string';
}

function isRegion(x: any): x is Nexus.Region {
  return typeof x?.isGlobal === 'boolean';
}

function decomposeRegion(region: Nexus.Region): RegionItem[] {
  if(region.isGlobal) {
    return [region];
  }
  const countries = [];
  for(const country of region.countries) {
    countries.push(country);
  }
  countries.sort((a: Nexus.CountryCode, b: Nexus.CountryCode) => {
    const aCode = Nexus.defaultCountryDatabase.fromCode(a).twoLetterCode;
    const bCode = Nexus.defaultCountryDatabase.fromCode(b).twoLetterCode;
    return aCode.localeCompare(bCode);
  });
  const venues = [];
  for(const venue of region.venues) {
    venues.push(venue);
  }
  venues.sort((a: Nexus.Venue, b: Nexus.Venue) => {
    const aName = Nexus.defaultVenueDatabase.fromVenue(a).displayName;
    const bName = Nexus.defaultVenueDatabase.fromVenue(b).displayName;
    return aName.localeCompare(bName);
  });
  const securities = [];
  for(const security of region.securities) {
    securities.push(security);
  }
  securities.sort((a: Nexus.Security, b: Nexus.Security) => {
    return a.toString().localeCompare(b.toString());
  })
  return [].concat(countries).concat(venues).concat(securities);
}

function recomposeRegion(members: RegionItem[]): Nexus.Region {
  const region = new Nexus.Region();
  for(const member of members) {
    if(isRegion(member)) {
      region.add(member);
    } else {
      region.add(new Nexus.Region(member as any));
    }
  }
  return region;
}

function makeRegionItemText(item: RegionItem): string {
  if(isCountryCode(item)) {
    return Nexus.defaultCountryDatabase.fromCode(item).twoLetterCode;
  } else if(isVenue(item)) {
    return Nexus.defaultVenueDatabase.fromVenue(item).displayName;
  } else if(isSecurity(item)) {
    return item.toString();
  } else {
    if(item.isGlobal) {
      return '*';
    }
    for(const country of item.countries) {
      return makeRegionItemText(country);
    }
    for(const venue of item.venues) {
      return makeRegionItemText(venue);
    }
    for(const security of item.securities) {
      return makeRegionItemText(security);
    }
  }
  return '';
}

function makeDisplayText(region: Nexus.Region): string {
  if(region.isGlobal) {
    return '*';
  }
  let text = '';
  const members = decomposeRegion(region);
  for(const member of members) {
    if(text !== '') {
      text += ", ";
    }
    text += makeRegionItemText(member);
  }
  return text;
}
