import * as React from 'react';

/** The properties used to render an HLine. */
interface Properties {

  /** The height of the line. */
  height?: number | string;

  /** The line's color. */
  color?: string;
}

/** Displays a horizontal line. */
export class HLine extends React.PureComponent<Properties> {
  public static defaultProps: Properties = {
    height: '1px',
    color: '#000000'
  }

  public render(): JSX.Element {
    const height = (() => {
      if(typeof(this.props.height) === 'number') {
        return `${this.props.height}px`;
      }
      return this.props.height;
    })();
    const style = {
      borderBottom: `${height} solid ${this.props.color}`
    } as React.CSSProperties;
    return <div style={style}/>;
  }
}
