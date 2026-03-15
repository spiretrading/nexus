import * as React from 'react';

interface Properties {

  /** The displayed message. */
  message: string;
}

/** Displays a centered message within an empty content area. */
export function EmptyMessage(props: Properties): JSX.Element {
  return (
    <div style={STYLE.container}>
      <div style={STYLE.filler}/>
      <span style={STYLE.message}>{props.message}</span>
      <div style={STYLE.filler}/>
    </div>);
}

const STYLE: Record<string, React.CSSProperties> = {
  container: {
    fontFamily: 'Roboto',
    backgroundColor: '#FFFFFF',
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center',
    height: '224px'
  },
  filler: {
    flexGrow: 1,
    flexShrink: 1,
    flexBasis: 0
  },
  message: {
    textAlign: 'center',
    color: '#333333',
    fontSize: '0.875rem'
  }
};
