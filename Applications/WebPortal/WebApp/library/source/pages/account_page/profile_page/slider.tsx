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

export class Slider extends React.Component<Properties, State> {

  constructor(properties: Properties) {
    super(properties);
    this.state = {
      value: 0
    };
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {

    return (
      <div>
        <div id='SLIDER' style={Slider.STYLE.containerStyle}>
          <div style={Slider.STYLE.circle} />
          <div style={Slider.STYLE.filler} />
          <HLine height='4px' color='#E6E6E6' />
          <div style={Slider.STYLE.filler} />
        </div>
        <div style={{height: '10px'}}/>
        <input type='range' min='1' max='100' value={this.state.value}
          onChange={this.onChange}
          className={css(Slider.SLIDER.slider)} />
      </div>);
  }

  private onChange() {
    this.setState({ value: this.state.value + 1 });
  }

  private onSlideLeft() {
    this.setState({ value: this.state.value });
  }

  private onSlideRight() {
    this.setState({ value: this.state.value });
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

  public static readonly SLIDER = StyleSheet.create({
    slider: {
      width: '100%',
      height: '20px',
      margin: 0,
      outline: 0,
      '::-webkit-slider-thumb': {
        boxSizing: 'border-box' as 'border-box',
        cursor: 'pointer' as 'pointer',
        height: '20px',
        width: '20px',
        backgroundColor: '#FFFFFF',
        border: '1px solid #8C8C8C',
        borderRadius: '20px'
      },
      '::-moz-range-thumb': {
        boxSizing: 'border-box' as 'border-box',
        cursor: 'pointer' as 'pointer',
        height: '20px',
        width: '20px',
        backgroundColor: '#FFFFFF',
        border: '1px solid #8C8C8C',
        borderRadius: '20px'
      },
      '::-ms-thumb': {
        boxSizing: 'border-box' as 'border-box',
        cursor: 'pointer' as 'pointer',
        height: '20px',
        width: '20px',
        backgroundColor: '#FFFFFF',
        border: '1px solid #8C8C8C',
        borderRadius: '20px'
      },
      '::-webkit-slider-runnable-track': {
        backgroundColor: '#E6E6E6',
        height: '4px'
      },
      '::-moz-range-track': {
        backgroundColor: '#E6E6E6',
        height: '4px'
      },
      '::-ms-track': {
        backgroundColor: '#E6E6E6',
        height: '4px'
      }
    }

  });
}
