import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';

export class GroupSelectionBox extends React.Component<> {

  public render(): JSX.Element {
    return <input type='text'
    className={css(GroupSelectionBox.DYNAMIC_STYLE.boxSmall)}/>;
  }

  private static DYNAMIC_STYLE = StyleSheet.create({
    boxSmall: {
      boxSizing: 'border-box' as 'border-box',
      paddingLeft: '10px',
      height: '34px',
      flexGrow: 1,
      minWidth: '284px'
    }
  });
}
