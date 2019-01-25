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
