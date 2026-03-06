import * as React from 'react';

interface Properties {

  /** The displayed message. */
  message: string;
}

/** Displays a centered message within an empty content area. */
export class EmptyMessage extends React.Component<Properties> {
  public render(): JSX.Element {
    return (
      <div style={EmptyMessage.STYLE.container}>
        <div style={EmptyMessage.STYLE.filler}/>
        <span style={EmptyMessage.STYLE.message}>{this.props.message}</span>
        <div style={EmptyMessage.STYLE.filler}/>
      </div>);
  }

  private static readonly STYLE = {
    container: {
      fontFamily: 'Roboto',
      backgroundColor: '#FFFFFF',
      display: 'flex',
      flexDirection: 'column',
      alignItems: 'center',
      height: '100%'
    } as React.CSSProperties,
    filler: {
      flexGrow: 1,
      flexShrink: 1,
      flexBasis: 0
    } as React.CSSProperties,
    message: {
      textAlign: 'center',
      color: '#333333',
      fontSize: '0.875rem'
    } as React.CSSProperties
  };
}
