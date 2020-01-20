import * as React from 'react';
import { DisplaySize } from '../display_size';

interface Properties {
  displaySize: DisplaySize;
}

export class Modal extends React.Component<Properties> {
  public render(): JSX.Element {
    const modalStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return Modal.STYLE.modalSmall;
      } else {
        return Modal.STYLE.modalLarge;
      }
    })();
    const modalWrapperStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return Modal.STYLE.modalWrapperSmall;
      } else {
        return Modal.STYLE.modalWrapperLarge;
      }
    })();
    const rightFillerStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return null;
      } else {
        return Modal.STYLE.filler;
      }
    })();
    return (
      <div style={Modal.STYLE.wrapper}>
        <div style={modalWrapperStyle}>
          <div style={Modal.STYLE.filler}/>
          <div style={modalStyle}>
            {this.props.children}
          </div>
          <div style={rightFillerStyle}/>
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
      backgroundColor: 'salmon',
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
    modalWrapperSmall: {
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
      flexDirection: 'row' as 'row',
      zIndex: 100000000
    },
    modalSmall: {
      boxSizing: 'border-box' as 'border-box',
      padding: '18px',
      backgroundColor: '#FFFFFF',
      boxShadow: '0px 0px 6px #00000066',
      height: '100%',
      width: '282px',
      alignSelf: 'flex-end' as 'center',
      flexGrow: 0,
      flexShrink: 0
    },
    modalWrapperLarge: {
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
    modalLarge: {
      boxSizing: 'border-box' as 'border-box',
      padding: '18px',
      backgroundColor: '#FFFFFF',
      boxShadow: '0px 0px 6px #00000066',
      height: '500px',
      width: '300px',
      alignSelf: 'center' as 'center',
      flexGrow: 0,
      flexShrink: 0
    }
  };
}
