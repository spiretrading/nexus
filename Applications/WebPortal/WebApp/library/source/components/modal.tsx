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
          <div style={{...Modal.STYLE.modal}}>
            {this.props.children}
          </div>
        </div>
        <div style={Modal.STYLE.overlay}id='overlay'/>
      </div>);
  }

  private static readonly STYLE = {
    wrapper: {
      height: '100%',
      width: '100%',
    },
    overlay: {
      height: '100%',
      width: '100%',
      zIndex: 9999000,
      backgroundColor: '#FFFFFF',
      opacity: '0.9',
      position: 'fixed' as 'fixed',
      margin: 0,
      padding: 0,
      top: 0,
      bottom: 0,
      left: 0,
      right: 0
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
      //display: 'flex' as 'flex',
      //flexWrap: 'nowrap' as 'nowrap',
      //justifyContent: 'center' as 'center',
      //alignItems: 'center' as 'center',
      zIndex: 100000000
    },
    modal: {
      marginTop: '20px',
      marginBottom: '20px',
      padding: '18px',
      backgroundColor: '#FFFFFF',
      boxShadow: '0px 0px 6px #00000066',
      height: '500px',
      width: '300px'
      //position: 'fixed' as 'fixed',
      //top: '50%',
      //bottom: '1%',
      //left: '50%',
      //right: '1%',
      //transform: 'transformation(50% 50%)'
    }
  };

}