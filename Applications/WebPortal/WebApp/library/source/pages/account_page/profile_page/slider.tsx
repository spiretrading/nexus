import * as React from 'react';
import { css, StyleSheet } from 'aphrodite/no-important';
import { HLine } from '../../../components';

interface Properties {

  /** The onClick event handler. */
  onSlide?: () => void;

  getValue?: () => void;
}

interface State {
  value: number;
}

export class Slider extends React.Component<Properties, {}> {

  constructor(properties: Properties) {
    super(properties);
    this.state = {
      isExpanded: false,
      isFirstTime: true
    };
  }

  public render(): JSX.Element {

    return (
      <div id='SLIDER' style={Slider.STYLE.containerStyle}>
        <div style={Slider.STYLE.circle}
          />
        <div style={Slider.STYLE.filler}/>
        <HLine height='4px' color='#E6E6E6'/>
        <div style={Slider.STYLE.filler}/>
      </div>);
  }

  public static readonly STYLE = {
    containerStyle: {
      position: 'relative' as 'relative',
      width: '100%',
      height: '20px'
    },
    filler: {
      height: '8px'
    },
    circle: {
      boxSizing: 'border-box' as 'border-box',
      position: 'absolute' as 'absolute',
      cursor: 'pointer' as 'pointer',
      zIndex: 1,
      height: '20px',
      width: '20px',
      backgroundColor: '#FFFFFF',
      border: '1px solid #8C8C8C',
      borderRadius: '20px'
    }
  };
}
