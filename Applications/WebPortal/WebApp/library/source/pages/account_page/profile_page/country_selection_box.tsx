import * as Nexus from 'nexus';
import * as React from 'react';
import { HBoxLayout, VBoxLayout, Padding, DisplaySize } from '../../..';

interface Properties {

  /** Determines if the input is editable. */
  readonly?: boolean;

  /**  */
  countryDatabase: Nexus.CountryDatabase;

  value?: string;

  onChange: (newCountry: string) => void;

  displaySize: DisplaySize;
}

interface State {
  options: any[]; // better type
}

/** Displays an input form for a single item. */
export class CountrySelectionBox extends React.Component<Properties, State> {
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
    return (
      <select value={this.props.value || ''}
          onChange={(event: React.ChangeEvent<HTMLSelectElement>) => {
            this.props.onChange(event.target.value);
          }}>
        {this.state.options}
      </select>);
  }

  private generateList() {
    for (const country of this.props.countryDatabase) {
      this.state.options.push(
        <option value={country.name}>
          {country.name}
        </option>);
    }
  }

  private static STYLE = {
    boxSmall: {
      width: '100%'
    },
    boxLarge: {
      width: '200px'
    },
    horizontalHeader: {
      height: '34px',
      width: '130px',
      font: '400 14px Roboto',
      color: '#000000',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center'
    },
    verticalHeader : {
      height: '16px',
      font: '400 14px Roboto',
      color: '#000000',
      paddingLeft: '10px'
    }
  };

}
