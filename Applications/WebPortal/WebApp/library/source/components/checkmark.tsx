import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { DisplaySize } from '../display_size';

interface Properties {

  displaySize?: DisplaySize;

  onClick?: (event?: React.MouseEvent<any>) => void;

  isSelected: boolean;

  readonly: boolean;
}

export class Checkmark extends React.Component<Properties> {
  public static readonly defaultProps = {
    onClick: () => {},
    readonly: false
  }

  public render(): JSX.Element {
    const imgSrc = (() => {
      if(this.props.isSelected) {
        return 'resources/components/check-green.svg';
      } else {
        return 'resources/components/check-grey.svg';
      }
    })();
    const size = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return '20px';
      } else {
        return '16px';
      }
    })();
    const wrapperStyle = (() => {
      if(this.props.readonly) {
        return Checkmark.STYLE.wrapperReadonly;
      } else {
        return Checkmark.STYLE.wrapper;
      }
    })();
    return (
      <div style={wrapperStyle}
          className={css(Checkmark.EXTRA_STYLE.removingDefaults)} onClick={this.props.onClick}>
        <img height={size} width={size} src={imgSrc}/> 
      </div>);
  }

  private static readonly STYLE = {
    wrapper: {
      boxSizing: 'border-box' as 'border-box',
      height: '20px',
      width: '20px',
      display: 'flex' as 'flex',
      alignItems: 'center' as 'center',
      alignContent: 'center' as 'center',
      justifyContent: 'center' as 'center',
      cursor: 'pointer' as 'pointer'
    },
    wrapperReadonly: {
      boxSizing: 'border-box' as 'border-box',
      height: '20px',
      width: '20px',
      display: 'flex' as 'flex',
      alignItems: 'center' as 'center',
      alignContent: 'center' as 'center',
      justifyContent: 'center' as 'center',
      cursor: 'default' as 'default'
    }
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    removingDefaults: {
      '-webkit-appearance': 'none',
      ':active': {
        ouline: 'none',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      ':focus': {
        ouline: 'none',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      '::moz-focus-inner': {
        border: 0
      },
      ':-moz-focusring': {
        color: 'transparent',
        textShadow: '0 0 0 #000'
      }
    }
  });
}