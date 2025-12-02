import { css, StyleSheet } from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** The current value of the input field. */
  value: string;

  /**
   * Called when the displayed value changes.
   * @param value - The new value.
   */
  onChange: (value: string) => void;

  /**
   * Called when the value is submitted.
   * @param value - The region.
   */
  onEnter: (value: Nexus.Region) => void;
}

/** Renders an input field for a single region item. */
export class RegionItemInput extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
  }

  public render() {
    return (<input
      className={css(RegionItemInput.EXTRA_STYLE.effects)}
      style={RegionItemInput.STYLE.input}
      placeholder={RegionItemInput.PLACEHOLDER_TEXT}
      onChange={this.onInputChange}
      onKeyDown={this.onKeyDown}
      value={this.props.value}/>);
  }

  private onInputChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.props.onChange(event.target.value);
  }

  private onKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    if(event.key === 'Enter') {
      const name = this.props.value.trim().toUpperCase();
      if(name === '*') {
        this.props.onEnter(Nexus.Region.makeGlobal('*'));
        return;
      }
      const country = parseCountry(name);
      if(!country.equals(Nexus.CountryCode.NONE)) {
        this.props.onEnter(new Nexus.Region(country));
        return;
      }
      const venue = parseVenue(name);
      if(!venue.equals(Nexus.Venue.NONE)) {
        this.props.onEnter(new Nexus.Region(venue));
        return;
      }
      const security = Nexus.Security.parse(name);
      if(!security.equals(Nexus.Security.NONE)) {
        this.props.onEnter(new Nexus.Region(security));
        return;
      }
    }
  }

  private static readonly STYLE = {
    input: {
      width: '100%',
      boxSizing: 'border-box',
      font: '400 14px Roboto',
      height: '34px',
      paddingLeft: '10px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      marginBottom: '18px'
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
    }
  });

  private static readonly PLACEHOLDER_TEXT = 'Region';
}

function parseCountry(text: string): Nexus.CountryCode {
  for(const country of Nexus.defaultCountryDatabase) {
    if(text === country.twoLetterCode || text === country.threeLetterCode ||
        text === country.name.toUpperCase()) {
      return country.code;
    }
  }
  return Nexus.CountryCode.NONE;
}

function parseVenue(text: string): Nexus.Venue {
  for(const venue of Nexus.defaultVenueDatabase) {
    if(text === venue.displayName.toUpperCase() ||
        text === venue.venue.toString()) {
      return venue.venue;
    }
  }
  return Nexus.Venue.NONE;
}
