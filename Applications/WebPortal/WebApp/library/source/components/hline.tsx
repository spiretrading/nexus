import * as React from 'react';

interface Properties {

  /** The height of the line. */
  height?: number | string;

  /** The line's color. */
  color?: string;
}

/** Displays a horizontal line. */
export function HLine(props: Properties): JSX.Element {
  const height = (() => {
    const h = props.height ?? '1px';
    if(typeof h === 'number') {
      return `${h}px`;
    }
    return h;
  })();
  const style: React.CSSProperties = {
    borderBottom: `${height} solid ${props.color ?? '#000000'}`
  };
  return <div style={style}/>;
}
