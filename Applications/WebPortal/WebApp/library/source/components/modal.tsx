import * as React from 'react';
import { DisplaySize } from '..';

interface Properties {

  /** The size of the viewport. */
  displaySize: DisplaySize;

  /** The height of the modal. */
  height: string | number;

  /** The width of the modal. */
  width: string;

  /** Called when the modal should be closed. */
  onClose?: () => void;
}

/** This is a component that wraps a child component to style it as a modal. */
export class Modal extends React.Component<Properties> {
  public static readonly defaultProps = {
    onClose: () => {}
  };

  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    const modalStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return Modal.STYLE.modalSmall;
      } else {
        return {...Modal.STYLE.modalLarge,
          width: this.props.width, height: this.props.height};
      }
    })();
    const modalWrapperStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return Modal.STYLE.modalWrapperSmall;
      } else {
        return Modal.STYLE.modalWrapperLarge;
      }
    })();
    const bottomPadding = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return null;
      } else {
        return Modal.STYLE.filler;
      }
    })();
    return (
      <div style={Modal.STYLE.wrapper}>
        <div style={modalWrapperStyle}onClick={this.outOfBoundsClick}>
          <div style={Modal.STYLE.filler} onClick={this.props.onClose}/>
          <div style={modalStyle}>
            {this.props.children}
          </div>
          <div style={bottomPadding} onClick={this.props.onClose}/>
        </div>
        <div style={Modal.STYLE.overlay} onClick={this.props.onClose}/>
      </div>);
  }

  private outOfBoundsClick = (event: React.MouseEvent<HTMLDivElement>) => {
    if(event.target === event.currentTarget) {
      this.props.onClose();
    }
  }

  private static readonly STYLE = {
    wrapper: {
      height: '100%',
      width: '100%',
      position: 'fixed',
      zIndex: 9998000
    } as React.CSSProperties,
    filler: {
      flexBasis: '20px',
      flexGrow: 1,
      flexShrink: 0
    } as React.CSSProperties,
    overlay: {
      height: '100%',
      width: '100%',
      backgroundColor: '#FFFFFF',
      opacity: '0.9',
      position: 'fixed',
      margin: 0,
      padding: 0,
      top: 0,
      bottom: 0,
      left: 0,
      right: 0,
      zIndex: 9998000
    } as React.CSSProperties,
    modalWrapperSmall: {
      overflowY: 'auto',
      overflowX: 'hidden',
      height: '100%',
      maxHeight: '100%',
      width: '100%',
      position: 'fixed',
      top: 0,
      bottom: 0,
      left: 0,
      right: 0,
      margin: 0,
      padding: 0,
      display: 'flex',
      flexWrap: 'nowrap',
      flexDirection: 'row',
      zIndex: 100000000
    } as React.CSSProperties,
    modalSmall: {
      boxSizing: 'border-box',
      paddingBottom: '40px',
      backgroundColor: '#FFFFFF',
      boxShadow: '0px 0px 6px #00000066',
      height: '100%',
      display: 'flex',
      flexWrap: 'nowrap',
      flexDirection: 'column',
      justifyContent: 'flex-start',
      flexGrow: 0,
      flexShrink: 0
    } as React.CSSProperties,
    modalWrapperLarge: {
      overflowY: 'auto',
      overflowX: 'hidden',
      height: '100%',
      maxHeight: '100%',
      width: '100%',
      position: 'fixed',
      top: 0,
      bottom: 0,
      left: 0,
      right: 0,
      margin: 0,
      padding: 0,
      display: 'flex',
      flexWrap: 'nowrap',
      flexDirection: 'column',
      zIndex: 100000000
    } as React.CSSProperties,
    modalLarge: {
      boxSizing: 'border-box',
      backgroundColor: '#FFFFFF',
      boxShadow: '0px 0px 6px #00000066',
      alignSelf: 'center',
      flexGrow: 0,
      flexShrink: 0
    }  as React.CSSProperties
  };
}
