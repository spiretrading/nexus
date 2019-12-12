import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '../display_size';
import { DurationInputField } from './duration_input_field';

interface Properties {

  /** The size to display the component at. */
  displaySize: DisplaySize;

  /** The value to display in the field. */
  value?: Beam.Duration;

  newValue?: Beam.DateTime;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange: (value: Beam.DateTime) => void;
}

/** A component that displays and lets a user edit a duration. */
export class DateTimeField extends React.Component<Properties> {
  public static readonly defaultProps = {
    value: new Beam.Duration(0),
    onChange: () => {}
  };

  public render(): JSX.Element {
    const wrapperStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return DateTimeField.STYLE.wrapperSmall;
      } else {
        return DateTimeField.STYLE.wrapperLarge;
      }
    })();
    return (
      <div style={wrapperStyle}>
        <div>{'DATE'}</div>
        <DurationInputField 
          displaySize={this.props.displaySize}
          value={this.props.newValue.timeOfDay()}
          />
      </div>);
  }

  private static readonly STYLE = {
    wrapperSmall: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      minWidth: '184px',
      width: '100%',
      flexShrink: 1,
      flexGrow: 1,
      backgroundColor: '#ffffff',
      justifyContent: 'space-between' as 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      height: '32px'
    },
    wrapperLarge: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexGrow: 1,
      flexShrink: 1,
      maxWidth: '246px',
      backgroundColor: '#ffffff',
      justifyContent: 'space-between' as 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      height: '32px'
    },
    inner: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexGrow: 1,
      justifyContent: 'flex-start' as 'flex-start',
      alignItems: 'center',
      marginLeft: '9px' 
    },
    integerBoxSmall: {
      boxSizing: 'border-box' as 'border-box',
      font: '400 14px Roboto',
      minWidth: '16px',
      maxWidth: '16px',
      width: '100%',
      height: '17px',
      flexGrow: 1,
      flexShrink: 1,
      border: '0px solid #ffffff',
      padding: 0
    },
    integerBoxLarge: {
      boxSizing: 'border-box' as 'border-box',
      font: '400 14px Roboto',
      minWidth: '16px',
      maxWidth: '16px',
      height: '17px',
      border: '0px solid #ffffff',
      padding: 0
    },
    colon: {
      width: '10px',
      height: '16px',
      flexGrow: 0,
      flexShrink: 0,
      display: 'flex' as 'flex',
      justifyContent: 'center',
      alignItems: 'center',
    },
    placeholder: {
      font: '500 11px Roboto',
      color: '#8C8C8C',
      display: 'flex' as 'flex',
      justifyContent: 'center',
      alignItems: 'center',
      marginRight: '10px'
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
