import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';

interface Properties {

  /** The value of the filter. */
  value: string;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange: (value: string) => void;
}

/** Filter bar for the account directory page. */
export class FilterBar extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {
    return (
      <div className={css(FilterBar.DYNAMIC_STYLE.box)}>
        <input value={this.props.value} placeholder='Filter'
          onChange={this.onChange}
          className={css(FilterBar.DYNAMIC_STYLE.input)}/>
        <img src={'resources/account_directory_page/magnifying-glass.svg'}
          style={FilterBar.STYLE.image}/>
      </div>);
  }

  private onChange(event: React.ChangeEvent<HTMLInputElement>) {
    this.props.onChange(event.target.value);
  }

  private static readonly STYLE = {
    image: {
      height: '16px',
      width: '16px',
      paddingRight: '10px',
      flex: '0 0 auto'
    }
  };
  private static readonly DYNAMIC_STYLE = StyleSheet.create({
    box: {
      boxSizing: 'border-box' as 'border-box',
      width: '100%',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      justifyContent: 'space-between' as 'space-between',
      alignItems: 'center' as 'center',
      borderRadius: '1px',
      border: '1px solid #C8C8C8',
      ':focus-within': {
        border: '1px solid #684BC7'
      }
    },
    input: {
      boxSizing: 'border-box' as 'border-box',
      font: '400 14px Roboto',
      color: '#000000',
      flexGrow: 1,
      flexShrink: 1,
      padding: 0,
      marginLeft: '10px',
      outline: 0,
      outlineOffset: 0,
      border: 0,
      '-webkit-appearance': 'none',
      '::-ms-clear': {
        display: 'none' as 'none'
      }
    },
    '::placeholder': {
      color: '#8C8C8C'
    },
    ':focus': {
      outlineOffset: 0,
      outline: 0,
      border: 0,
      outlineColor: 'transparent',
      outlineStyle: 'none'
    },
    '::moz-focus-inner': {
      border: 0
    },
    '::-ms-clear': {
      display: 'none' as 'none'
    }
  });
}
