import * as React from 'react';

/** Returns a component that monitors the resize event.
 * @param BaseComponent - A component that needs some scrolly stuff
 */
export function pageWrapper(BaseComponent: React.ComponentType<any>) {
  return class extends React.Component {

    public render() {
      return (
        <div style={this.STYLE.page}> 
          <BaseComponent {...this.props}/>
        </div>);
    }
  
    STYLE = {
      page: {
        //boxSizing: 'border-box' as 'border-box',
        //width: '100%',
        //height: '100%',
        display: 'flex' as 'flex',
        flex: 'column' as 'column',
        flexWrap: 'nowrap' as 'nowrap',
        overflowY: 'auto' as 'auto',
        overflowX: 'hidden' as 'hidden',
        justifyContent: 'center' as 'center',
        alignItems: 'flex-start' as 'flex-start',
        margin: 0,
        padding: 0,
        minWidth: 0
      //minWidth: '320px'
      }
    };
  };
}
