import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { DisplaySize } from '../../../display_size';

interface Properties {
  onChange?: () => void;
  value?: string;
  displaySize: DisplaySize;
}

export class RuleExecutionDropDown extends React.Component<Properties> {
  constructor() {
    super(null);
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {
    const sizeStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return RuleExecutionDropDown.STYLE.boxSmall;
      } else {
        return RuleExecutionDropDown.STYLE.boxLarge;
      }
    })();
    return (
      <select onChange={this.onChange} value={this.props.value}
          style={{...sizeStyle,
            ...RuleExecutionDropDown.STYLE.selectionBoxStyle}}>
        <option value=''>{'Active'}</option>
        <option value=''>{'Active Per Account'}</option>
        <option value=''>{'Active Consolodated'}</option>
        <option value=''>{'Passive'}</option>
      </select>);
  }

  public onChange(event: React.ChangeEvent<HTMLSelectElement>): void {
  }

  private static readonly STYLE = {
    boxSmall: {
      height: '34px',
      font: '400 16px Roboto',
      width: '100%',
      minWidth: '246px',
      maxWidth: '386px'
    },
    boxLarge: {
      width: '200px',
      height: '34px',
      font: '400 14px Roboto'
    },
    selectionBoxStyle: {
      boxSizing: 'border-box' as 'border-box',
      paddingLeft: '7px',
      color: '#000000',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      backgroundColor: '#F2F2F2',
      backgroundImage:
        'url(resources/account_page/profile_page/arrow-down.svg)',
      backgroundRepeat: 'no-repeat',
      backgroundPosition: 'right 10px top 50%',
      backgroundSize: '8px 6px',
      MozAppearance: 'none' as 'none',
      WebkitAppearance: 'none' as 'none',
      appearance: 'none' as 'none'
    }
  };
  public static readonly EXTRA_STYLE = StyleSheet.create({
    noHighlighting: {
      ':focus': {
        ouline: 0,
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
