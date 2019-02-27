import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { DisplaySize } from '../../display_size';

interface Properties {
  displaySize: DisplaySize;
  error?: boolean;
  //some sort of groups
  //some sort of model
}

export class GroupSelectionBox extends React.Component<Properties> {

  public render(): JSX.Element {
    const boxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return GroupSelectionBox.DYNAMIC_STYLE.boxSmall;
      } else if(this.props.displaySize === DisplaySize.MEDIUM) {
        return GroupSelectionBox.DYNAMIC_STYLE.boxMedium;
      } else {
        return GroupSelectionBox.DYNAMIC_STYLE.boxLarge;
      }
    })();
    const errorBoxStyle = (() => {
      if(this.props.error) {
        return GroupSelectionBox.DYNAMIC_STYLE.error;
      } else {
        return null;
      }
    })();
    return (
      <div id='GROUP BOX'>
        <input type='text'
          className={css(boxStyle, errorBoxStyle,
          GroupSelectionBox.DYNAMIC_STYLE.text)}/>
      </div>);
  }

  private static DYNAMIC_STYLE = StyleSheet.create({
    boxSmall: {
      boxSizing: 'border-box' as 'border-box',
      paddingLeft: '10px',
      height: '34px',
      flexGrow: 1,
      minWidth: '284px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      ':focus': {
        borderColor: '#684BC7',
        ouline: 'none',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      ':active': {
        borderColor: '#684BC7'
      },
      '::moz-focus-inner': {
        border: 0
      }
    },
    boxMedium: {
      boxSizing: 'border-box' as 'border-box',
      paddingLeft: '10px',
      height: '34px',
      flexGrow: 1,
      minWidth: '284px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      ':focus': {
        borderColor: '#684BC7',
        ouline: 'none',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      ':active': {
        borderColor: '#684BC7'
      },
      '::moz-focus-inner': {
        border: 0
      }
    },
    boxLarge: {
      boxSizing: 'border-box' as 'border-box',
      paddingLeft: '10px',
      height: '34px',
      flexGrow: 1,
      minWidth: '350px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      ':focus': {
        borderColor: '#684BC7',
        ouline: 'none',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      ':active': {
        borderColor: '#684BC7'
      },
      '::moz-focus-inner': {
        border: 0
      }
    },
    text: {
      font: '400 14px Roboto',
      color: '#000000'
    },
    error: {
      border: '1px solid #E63F44'
    }
  });
}

// Suggestion Window
