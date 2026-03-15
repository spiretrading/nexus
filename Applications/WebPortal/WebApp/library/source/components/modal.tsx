import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties {

  /** The title displayed in the modal header. If provided, a header with a
   *  close button is rendered. */
  title?: string;

  /** Called when the modal should be closed. */
  onClose?: () => void;

  children?: React.ReactNode;
}

/** This is a component that wraps a child component to style it as a modal. */
export function Modal(props: Properties): JSX.Element {
  const outOfBoundsClick = (event: React.MouseEvent<HTMLDivElement>) => {
    if(event.target === event.currentTarget) {
      props.onClose?.();
    }
  };
  const header = (() => {
    if(props.title === undefined) {
      return null;
    }
    return (
      <div style={STYLE.header}>
        <div style={STYLE.headerTitle}>{props.title}</div>
        <img src='resources/close.svg'
          style={STYLE.closeButton}
          height='20px' width='20px'
          onClick={props.onClose}/>
      </div>);
  })();
  return (
    <div style={STYLE.wrapper}>
      <div className={css(STYLES.smallModal, STYLES.modalBody)}>
        {header}
        {props.children}
      </div>
      <div className={css(STYLES.largeModal)} onClick={outOfBoundsClick}>
        <div style={STYLE.filler} onClick={props.onClose}/>
        <div className={css(STYLES.modalBody)} style={STYLE.modalLarge}>
          {header}
          {props.children}
        </div>
        <div style={STYLE.filler} onClick={props.onClose}/>
      </div>
      <div style={STYLE.overlay} onClick={props.onClose}/>
    </div>);
}

const STYLE: Record<string, React.CSSProperties> = {
  wrapper: {
    position: 'fixed',
    top: 0,
    left: 0,
    right: 0,
    bottom: 0,
    zIndex: 9998000
  },
  filler: {
    flexBasis: '20px',
    flexGrow: 1,
    flexShrink: 0,
    width: '100%'
  },
  overlay: {
    position: 'fixed',
    top: 0,
    left: 0,
    right: 0,
    bottom: 0,
    backgroundColor: '#FFFFFF',
    opacity: 0.9,
    zIndex: 9999000
  },
  modalLarge: {
    boxSizing: 'border-box',
    flexGrow: 0,
    flexShrink: 0
  },
  header: {
    boxSizing: 'border-box',
    display: 'flex',
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    padding: '18px 18px 0 18px',
    flexGrow: 0,
    flexShrink: 0
  },
  headerTitle: {
    font: '400 16px Roboto',
    color: '#333333',
    cursor: 'default'
  },
  closeButton: {
    cursor: 'pointer',
    flexShrink: 0
  }
};

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
  },
  smallModal: {
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
    zIndex: 10000000,
    '@media (min-width: 768px)': {
      display: 'none'
    }
  },
  largeModal: {
    position: 'fixed',
    top: 0,
    left: 0,
    right: 0,
    bottom: 0,
    display: 'none',
    flexDirection: 'column',
    alignItems: 'center',
    overflowY: 'auto',
    overflowX: 'hidden',
    zIndex: 10000000,
    '@media (min-width: 768px)': {
      display: 'flex'
    }
  }
});
