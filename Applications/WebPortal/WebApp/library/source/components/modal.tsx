import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { DisplaySize } from '..';

interface Properties {

  /** The size of the viewport. */
  displaySize: DisplaySize;

  /** The title displayed in the modal header. If provided, a header with a
   *  close button is rendered. */
  title?: string;

  /** Called when the modal should be closed. */
  onClose?: () => void;
}

/** This is a component that wraps a child component to style it as a modal. */
export class Modal extends React.Component<Properties> {
  public render(): JSX.Element {
    const header = (() => {
      if(this.props.title === undefined) {
        return null;
      }
      return (
        <div style={Modal.STYLE.header}>
          <div style={Modal.STYLE.headerTitle}>{this.props.title}</div>
          <img src='resources/close.svg'
            style={Modal.STYLE.closeButton}
            height='20px' width='20px'
            onClick={this.props.onClose}/>
        </div>);
    })();
    if(this.props.displaySize === DisplaySize.SMALL) {
      return (
        <div style={Modal.STYLE.wrapper}>
          <div className={css(STYLES.modalBody)}
              style={Modal.STYLE.modalSmall}>
            {header}
            {this.props.children}
          </div>
          <div style={Modal.STYLE.overlay} onClick={this.props.onClose}/>
        </div>);
    }
    return (
      <div style={Modal.STYLE.wrapper}>
        <div style={Modal.STYLE.modalWrapper} onClick={this.outOfBoundsClick}>
          <div style={Modal.STYLE.filler} onClick={this.props.onClose}/>
          <div className={css(STYLES.modalBody)}
              style={Modal.STYLE.modalLarge}>
            {header}
            {this.props.children}
          </div>
          <div style={Modal.STYLE.filler} onClick={this.props.onClose}/>
        </div>
        <div style={Modal.STYLE.overlay} onClick={this.props.onClose}/>
      </div>);
  }

  private outOfBoundsClick = (event: React.MouseEvent<HTMLDivElement>) => {
    if(event.target === event.currentTarget) {
      this.props.onClose?.();
    }
  }

  private static readonly STYLE = {
    wrapper: {
      position: 'fixed',
      top: 0,
      left: 0,
      right: 0,
      bottom: 0,
      zIndex: 9998000
    } as React.CSSProperties,
    filler: {
      flexBasis: '20px',
      flexGrow: 1,
      flexShrink: 0,
      width: '100%'
    } as React.CSSProperties,
    overlay: {
      position: 'fixed',
      top: 0,
      left: 0,
      right: 0,
      bottom: 0,
      backgroundColor: '#FFFFFF',
      opacity: 0.9,
      zIndex: 9999000
    } as React.CSSProperties,
    modalWrapper: {
      position: 'fixed',
      top: 0,
      left: 0,
      right: 0,
      bottom: 0,
      display: 'flex',
      flexDirection: 'column',
      alignItems: 'center',
      overflowY: 'auto',
      overflowX: 'hidden',
      zIndex: 10000000
    } as React.CSSProperties,
    modalSmall: {
      position: 'fixed',
      top: 0,
      right: 0,
      bottom: 0,
      width: '282px',
      boxSizing: 'border-box',
      display: 'flex',
      flexDirection: 'column',
      overflow: 'auto',
      paddingBottom: '40px',
      zIndex: 10000000
    } as React.CSSProperties,
    modalLarge: {
      boxSizing: 'border-box',
      flexGrow: 0,
      flexShrink: 0
    } as React.CSSProperties,
    header: {
      boxSizing: 'border-box',
      display: 'flex',
      flexDirection: 'row',
      justifyContent: 'space-between',
      alignItems: 'center',
      padding: '18px 18px 0 18px',
      flexGrow: 0,
      flexShrink: 0
    } as React.CSSProperties,
    headerTitle: {
      font: '400 16px Roboto',
      color: '#333333',
      cursor: 'default'
    } as React.CSSProperties,
    closeButton: {
      cursor: 'pointer',
      flexShrink: 0
    } as React.CSSProperties
  };
}

const STYLES = StyleSheet.create({
  modalBody: {
    backgroundColor: '#FFFFFF',
    boxShadow: '-3px 0 6px rgb(0 0 0 / 40%)',
    borderRadius: '1px',
    color: '#333333',
    fontFamily: "'Roboto', system-ui, sans-serif",
    transition: 'opacity 200ms ease-in',
    '@media (min-width: 768px)': {
      boxShadow: '0 0 6px rgb(0 0 0 / 40%)'
    },
    '@starting-style': {
      opacity: 0
    }
  }
});
