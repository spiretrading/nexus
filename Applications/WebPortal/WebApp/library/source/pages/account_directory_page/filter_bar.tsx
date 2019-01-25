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

interface State {
  isFocused: boolean;
}

/** Filter bar for the account directory page. */
export class FilterBar extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isFocused: false
    };
    this.onBlur = this.onBlur.bind(this);
    this.onFocus = this.onFocus.bind(this);
  }

  public render(): JSX.Element {
    const boxStyle = (() => {
      if(this.state.isFocused) {
        return FilterBar.STYLE.activeBox;
      } else {
        return FilterBar.STYLE.box;
      }
    })();
    return (
      <div style={boxStyle}>
        <input value={this.props.value} placeholder='Filter'
          onFocus={this.onFocus} onBlur={this.onBlur}
          onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
            this.props.onChange(event.target.value);
          }}
          className={css(FilterBar.DYNAMIC_STYLE.input)}/>
        <img src={'resources/account_directory_page/magnifying-glass.svg'}
          style={FilterBar.STYLE.image}/>
      </div>);
  }

  private onFocus() {
    this.setState({isFocused: true});
  }

  private onBlur() {
    this.setState({isFocused: false});
  }

  private static readonly STYLE = {
    box: {
      width: '100%',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      justifyContent: 'space-between' as 'space-between',
      alignItems: 'center' as 'center',
      borderRadius: '1px',
      border: '1px solid #C8C8C8'
    },
    activeBox: {
      width: '100%',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      justifyContent: 'space-between' as 'space-between',
      alignItems: 'center' as 'center',
      borderRadius: '1px',
      border: '1px solid #684BC7'
    },
    image: {
      height: '16px',
      width: '16px',
      paddingRight: '10px',
      flex: '0, 0, auto'
    }
  };

  private static DYNAMIC_STYLE = StyleSheet.create({
    input: {
      font: '400 14px Roboto',
      color: '#000000',
      flexGrow: 1,
      flexShrink: 1,
      paddingLeft: '10px',
      ouline: 0,
      border: 0
    },
    '::placeholder': {
      color: '#8C8C8C'
    },
    ':focus': {
      ouline: 0,
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
