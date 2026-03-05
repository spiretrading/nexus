import * as Beam from 'beam';
import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value?: Beam.DateTime;

  /** The callback to update the value. */
  update?: (newValue: Beam.DateTime) => void;
}

/** A datetime input that converts between Beam.DateTime and a
 *  datetime-local input. */
export class BeamDateTimeInput extends React.Component<Properties> {
  public render(): JSX.Element {
    const date = this.props.value.date;
    const time = this.props.value.timeOfDay.split();
    const year = String(date.year).padStart(4, '0');
    const month = String(date.month).padStart(2, '0');
    const day = String(date.day).padStart(2, '0');
    const hours = String(time.hours).padStart(2, '0');
    const minutes = String(time.minutes).padStart(2, '0');
    return <input type='datetime-local'
      value={`${year}-${month}-${day}T${hours}:${minutes}`}
      onChange={this.onChange}/>;
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const value = event.target.value;
    const [datePart, timePart] = value.split('T');
    if(!datePart || !timePart) {
      return;
    }
    const dateParts = datePart.split('-');
    const timeParts = timePart.split(':');
    if(dateParts.length !== 3 || timeParts.length < 2) {
      return;
    }
    const date = new Beam.Date(
      parseInt(dateParts[0]), parseInt(dateParts[1]), parseInt(dateParts[2]));
    const timeOfDay = Beam.Duration.HOUR.multiply(parseInt(timeParts[0])).add(
      Beam.Duration.MINUTE.multiply(parseInt(timeParts[1])));
    this.props.update(new Beam.DateTime(date, timeOfDay));
  }
}
