import * as React from 'react';

interface Properties {
  value: string;
  onChange: (newValue: string) => void;
}

export class FilterBar extends React.Component<Properties> {
  public render(): JSX.Element {
    return (<div style={FilterBar.STYLE.test}/>);
  }

  private static readonly STYLE = {
    test: {
      width: '100%',
      height: '34px',
      backgroundColor: '#4B23A0'
    }
  };
}
