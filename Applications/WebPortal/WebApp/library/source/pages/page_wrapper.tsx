import * as React from 'react';

/** This is a component that wraps every subpage so that the scrolling behavior 
    is consistent within all pages.*/
export class PageWrapper extends React.Component {
  public render(): JSX.Element {
    return (
      <div style={PageWrapper.STYLE.page}>
        {this.props.children}
      </div>);
  }

  private static readonly STYLE = {
    page: {
      backgroundColor: '#ffffff',
      width: '100%',
      height: '100%',
      display: 'flex' as 'flex',
      overflowX: 'hidden' as 'hidden',
      overflowY: 'auto' as 'auto',
      justifyContent: 'center' as 'center',
      alignItems: 'flex-start' as 'flex-start',
      margin: 0,
      padding: 0,
      minWidth: '320px'
    }
  };
}
