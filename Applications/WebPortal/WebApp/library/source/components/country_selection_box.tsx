import * as React from 'react';
import * as Nexus from 'nexus';
import { DisplaySize } from '../display_size';
import { NONAME } from 'dns';
import { url } from 'inspector';

interface Properties {

  /** The set of available countries to select. */
  countryDatabase: Nexus.CountryDatabase;

  /** The currently selected country. */
  value: Nexus.CountryCode;

  /** Whether the selection box is read only. */
  readonly?: boolean;

  /** The event handler called when the selection changes. */
  onChange?: (currency: Nexus.CountryCode) => void;

  displaySize: DisplaySize;
}

/** Displays a selection box for countries. */
interface State {
  options: any[]; // better type
}

/** Displays an input form for a single item. */
export class CountrySelectionBox extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    readonly: false
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      options: []
    };
    this.generateList();
  }

  public render(): JSX.Element {
    const boxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return CountrySelectionBox.STYLE.boxSmall;
      } else {
        return CountrySelectionBox.STYLE.boxLarge;
      }
    })();
    const selectStyle = (() => {
      if(this.props.readonly) {
        return CountrySelectionBox.STYLE.disabledBoxStyle;
      } else {
        return CountrySelectionBox.STYLE.selectionBoxStyle;
      }
    })();
    const dropDownArrowStyle = (() => {
    })();
    return (
      <select value={this.props.value.code || ''}
          disabled={this.props.readonly}
          style={{...boxStyle,...selectStyle}}>
          onChange={(event: React.ChangeEvent<HTMLSelectElement>) => {
            this.props.onChange(this.props.value);
          }}>
        {this.state.options}
      </select>);
  }

  private generateList() {
    for (const country of this.props.countryDatabase) {
      this.state.options.push(
        <option value={country.code.code}>
          {country.name}
        </option>);
    }
  }

  private static STYLE = {
    boxSmall: {
      maxWidth: '424px',
      height: '34px'
    },
    boxLarge: {
      width: '200px',
      height: '34px'
    },
    selectionBoxStyle: {
      boxSizing: 'border-box' as 'border-box',
      paddingLeft: '6px',
      color: '#000000',
      font: '400 16px Roboto',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      backgroundColor: '#F2F2F2',
      backgroundImage:
        'url(resources/account_page/profile_page/arrow-down.svg)',
      backgroundRepeat: 'no-repeat',
      backgroundPosition: 'right 10px top 50%',
      backgroundSize: '8px 6px',
      '-moz-appearance': 'none' as 'none',
      '-webkit-appearance': 'none' as 'none',
      appearance: 'none' as 'none'
    },
    disabledBoxStyle: {
      boxSizing: 'border-box' as 'border-box',
      paddingLeft: '6px',
      color: '#000000',
      font: '400 16px Roboto',
      backgroundColor: '#FFFFFF',
      border: '1px solid #FFFFFF',
      borderRadius: '1px',
      '-moz-appearance': 'none' as 'none',
      '-webkit-appearance': 'none' as 'none',
      appearance: 'none' as 'none'
    }
  };
  private static readonly ARROW_IMG_PATH =
    'resources/account_page/profile_page/arrow-down.svg';
}
