import * as React from 'react';

/** Returns a page wrapped in basic
 * @param BaseComponent - A component that needs proper scrolling behavior.
 */
export function pageWrapper(BaseComponent: React.ComponentType<any>) {
  return class extends React.Component {
    public render() {
      return (
        <div style={this.STYLE.page}> 
          <BaseComponent {...this.props}/>
        </div>);
    }
  
    readonly STYLE = {
      page: {
        boxSizing: 'border-box' as 'border-box',
        width: '100%',
        height: '100%',
        display: 'flex' as 'flex',
        flex: 'column' as 'column',
        flexWrap: 'nowrap' as 'nowrap',
        overflowX: 'auto' as 'auto',
        overflowY: 'auto' as 'auto',
        justifyContent: 'center' as 'center',
        alignItems: 'flex-start' as 'flex-start',
        margin: 0,
        padding: 0,
        minWidth: '320px'
      }
    };
  };
}
