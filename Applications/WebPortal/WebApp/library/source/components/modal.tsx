import * as React from 'react';

export class Modal extends React.Component {
  public render(): JSX.Element {
    return (
      <div style={Modal.STYLE.wrapper}>
        <div style={Modal.STYLE.modalWrapper}>
          <div style={Modal.STYLE.filler}/>
          <div style={Modal.STYLE.modal}>
            {this.props.children}
          </div>
          <div style={Modal.STYLE.filler}/>
        </div>
        <div style={Modal.STYLE.overlay}id='overlay'/>
      </div>);
  }

  private static readonly STYLE = {
    wrapper: {
      height: '100%',
      width: '100%',
      position: 'fixed' as 'fixed',
      zIndex: 9998000
    },
    filler: {
      flexGrow: 1,
      flexBasis: '20px',
      flexShrink: 0
    },
    overlay: {
      height: '100%',
      width: '100%',
      backgroundColor: '#FFFFFF',
      opacity: '0.9',
      position: 'fixed' as 'fixed',
      margin: 0,
      padding: 0,
      top: 0,
      bottom: 0,
      left: 0,
      right: 0,
      zIndex: 9998000
    },
    modalWrapper: {
      overflowY: 'auto' as 'auto',
      overflowX: 'hidden' as 'hidden',
      height: '100%',
      maxHeight: '100%',
      width: '100%',
      position: 'fixed' as 'fixed',
      top: 0,
      bottom: 0,
      left: 0,
      right: 0,
      margin: 0,
      padding: 0,
      display: 'flex' as 'flex',
      flexWrap: 'nowrap' as 'nowrap',
      flexDirection: 'column' as 'column',
      zIndex: 100000000
    },
    modal: {
      boxSizing: 'border-box' as 'border-box',
      padding: '18px',
      backgroundColor: '#FFFFFF',
      border: '1px solid red',
      boxShadow: '0px 0px 6px #00000066',
      height: '500px',
      width: '300px',
      alignSelf: 'center' as 'center',
      flexGrow: 0,
      flexShrink: 0
    }
  };
}
