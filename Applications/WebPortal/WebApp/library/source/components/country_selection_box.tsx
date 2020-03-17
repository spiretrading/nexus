import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import * as Nexus from 'nexus';
import { DisplaySize } from '../display_size';
import { TextInputField } from '.';

interface Properties {

  /** The set of available countries to select. */
  countryDatabase: Nexus.CountryDatabase;

  /** The currently selected country. */
  value: Nexus.CountryCode;

  /** Whether the selection box is read only. */
  readonly?: boolean;

  /** The event handler called when the selection changes. */
  onChange?: (currency: Nexus.CountryCode) => void;

  /** Determines the size of the element. */
  displaySize: DisplaySize;
}

/** Displays a country selection box. */
export class CountrySelectionBox extends React.Component<Properties> {
  public static readonly defaultProps = {
    onChange: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {
    const boxSizing = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return CountrySelectionBox.STYLE.boxSmall;
      } else {
        return CountrySelectionBox.STYLE.boxLarge;
      }
    })();
    const options = (() => {
      const options = [];
      for(const country of this.props.countryDatabase) {
        options.push(
          <option value={country.code.code} key={country.code.code}>
            {country.name}
          </option>);
      }
      return options;
    })();
    const content = (() => {
      if(this.props.readonly) {
        return (
          <TextInputField
            value={this.props.countryDatabase.fromCode(this.props.value).name}
            displaySize={this.props.displaySize}/>);
      } else {
        return (
          <select value={this.props.value.code}
              className={css(CountrySelectionBox.EXTRA_STYLE.noHighting)}
              style={{...boxSizing,
                ...CountrySelectionBox.STYLE.selectionBoxStyle}}
              onChange={this.onChange}>
            {options}
          </select>);
      }
    })();
    return (<div>{content}</div>);
  }

  private onChange(event: React.ChangeEvent<HTMLSelectElement>): void {
    const code = new Nexus.CountryCode(parseInt(event.target.value, 10));
    this.props.onChange(code);
  }

  private static readonly STYLE = {
    boxSmall: {
      height: '34px',
      font: '400 16px Roboto',
      width: '100%'
    },
    boxLarge: {
      width: '200px',
      height: '34px',
      font: '400 14px Roboto'
    },
    selectionBoxStyle: {
      boxSizing: 'border-box' as 'border-box',
      paddingLeft: '7px',
      color: '#333333',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      backgroundColor: '#F2F2F2',
      backgroundImage:
        'url(resources/components/arrow-down.svg)',
      backgroundRepeat: 'no-repeat',
      backgroundPosition: 'right 10px top 50%',
      backgroundSize: '8px 6px',
      MozAppearance: 'none' as 'none',
      WebkitAppearance: 'none' as 'none',
      appearance: 'none' as 'none'
    }
  };
  public static readonly EXTRA_STYLE = StyleSheet.create({
    noHighting: {
      ':focus': {
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      '::moz-focus-inner': {
        border: 0
      },
      ':-moz-focusring': {
        color: 'transparent',
        textShadow: '0 0 0 #000'
      },
      '-webkit-user-select': 'text',
      '-moz-user-select': 'text',
      '-ms-user-select': 'text',
      'user-select': 'text'
    }
  });
}
