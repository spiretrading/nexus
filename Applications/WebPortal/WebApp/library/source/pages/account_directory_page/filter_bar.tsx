import * as React from 'react';

interface Properties {
  value: string;
  onChange: (newValue: string) => void;
}

export class FilterBar extends React.Component<Properties> {
  public render(): JSX.Element {
    return (<div/>);
  }
}
