import * as React from 'react';
import { DisplaySize } from '../display_size';

interface Props {
  onClose?: () => {};
  height: number | string;
  width: number | string;
}

export class Modal extends React.Component<Props> {
  public render(): JSX.Element {
    return (
      <div>
        <div style={Modal.STYLE.modalWrapper}>
          <div style={{...Modal.STYLE.modal, 
            ...{height: this.props.height},
            ...{width: this.props.width}}}>
            {this.props.children}
          </div>
        </div>
        <div style={Modal.STYLE.overlay}id='overlay'/>
      </div>);
  }

  private static readonly STYLE = {
    overlay: {
      height: '100%',
      width: '100%',
      zIndex: 9999000,
      backgroundColor: '#FFFFFF',
      opacity: '0.9',
      position: 'fixed' as 'fixed',
      top: 0,
      bottom: 0,
      left: 0,
      right: 0
    },
    modalWrapper: {
      overflowY: 'auto' as 'auto',
      overflowX: 'hidden' as 'hidden',
      height: '100%',
      width: '100%',
      position: 'fixed' as 'fixed',
      top: 0,
      bottom: 0,
      left: 0,
      right: 0,
      display: 'flex' as 'flex',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center',
      padding: '20px',
      zIndex: 100000000
    },
    modal: {
      marginTop: '20px',
      marginBottom: '20px',
      padding: '18px',
      backgroundColor: '#FFFFFF',
      boxShadow: '0px 0px 6px #00000066',
      position: 'absolute' as 'absolute'
    }
  };

}