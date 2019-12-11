import { css, StyleSheet } from 'aphrodite/no-important';
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
      <div style={wrapperStyle} onClick={this.props.onClick}>
        <img height={size} width={size} src={imgSrc}/> 
      </div>);
  }

  private static readonly STYLE = {
    wrapper: {
      height: '20px',
      width: '20px',
      display: 'flex' as 'flex',
      alignItems: 'center' as 'center',
      alignContent: 'center' as 'center',
      justifyContent: 'center' as 'center',
      cursor: 'pointer' as 'pointer'
    },
    wrapperReadonly: {
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
      ':active' : {
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
}