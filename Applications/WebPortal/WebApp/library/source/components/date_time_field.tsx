import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '../display_size';
import { DurationInputField } from './duration_input_field';
import { DateField } from './date_field';

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
    newValue: new Beam.DateTime(new Beam.Date(0, 0, 0), new Beam.Duration(0)),
    onChange: () => {}
  };

  public render(): JSX.Element {
    return (
      <div style={DateTimeField.STYLE.outerWrapper}>
        <DateField
          displaySize={this.props.displaySize}
          value={this.props.newValue.date()}/>
        <div style={DateTimeField.STYLE.filler}/>  
        <DurationInputField 
          displaySize={this.props.displaySize}
          value={this.props.newValue.timeOfDay()}/>
      </div>);
  }

  private static readonly STYLE = {
    outerWrapper: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      height: '78px'
    },
    filler: {
      height: '10px',
      width: '100%'
    }
  }
}
