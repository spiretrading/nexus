import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '../display_size';
import { IntegerInputBox } from './integer_input_box';

enum TimeUnit {
  HOURS,
  MINUITES,
  SECONDS
}

interface Properties { 
  displaySize?: DisplaySize;
  value: Beam.Duration;
  onChange:(newValue: Beam.Duration) => void;
}

export class DurationInput extends React.Component<Properties> {
  public render(): JSX.Element {
    const integerInputStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return DurationInput.STYLE.integerBoxSmall;
      } else {
        return DurationInput.STYLE.integerBox;
      }
    })();
    return (
      <div style={DurationInput.STYLE.wrapper}>
          <IntegerInputBox
            className={css(DurationInput.EXTRA_STYLE.effects)}
            style={integerInputStyle}
            padding={2}/>
          <div style={DurationInput.STYLE.padding}>
            :
          </div>
          <IntegerInputBox
            className={css(DurationInput.EXTRA_STYLE.effects)}
            style={integerInputStyle}
            padding={2}/>
          <div style={DurationInput.STYLE.padding}>
            :
          </div>
          <IntegerInputBox
            className={css(DurationInput.EXTRA_STYLE.effects)}
            style={integerInputStyle}
            padding={2}/>
      </div>);
  }

  private onChange() {
  }

  private static readonly STYLE = {
    wrapper: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row'  as 'row',
      flexGrow: 1,
      maxWidth: '246px'
    },
    wrapperSmall: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row'  as 'row',
      minWidth: '184px',
      maxWidth: '246px'
    },
    integerBoxSmall: {
      boxSizing: 'border-box' as 'border-box',
      minWidth: '44px',
      //maxWidth: '64px',
      height: '34px',
      flexGrow: 1,
      flexShrink: 1
    },
    integerBox: {
      boxSizing: 'border-box' as 'border-box',
      width: '64px',
      height: '34px'
    },
    padding: {
      width: '27px',
      flexShrink: 0,
      display: 'flex' as 'flex',
      justifyContent: 'center',
      alignItems: 'center'
    }
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    effects: {
      ':focus': {
        ouline: 0,
        borderColor: '#684BC7',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      ':active' : {
        borderColor: '#684BC7'
      },
      '::moz-focus-inner': {
        border: 0
      },
      '::placeholder': {
        color: '#8C8C8C'
      }
    }
  });
}