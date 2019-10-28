import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { DisplaySize } from '../../../display_size';

interface Properties {
  displaySize: DisplaySize;
  value?: string;
}

export class SymbolInputField extends React.Component<Properties> {
  public static readonly defaultProps = {

  };

  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    return <input />;
  }

    private static STYLE = StyleSheet.create({
    boxSmall: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      maxWidth: '246px',
      minWidth: '184px',
      font: '400 14px Roboto',
    },
    boxLarge: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      maxWidth: '246px',
      font: '400 14px Roboto',
    }
  });

}
