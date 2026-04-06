import * as Beam from 'beam';
import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value?: Beam.Date;

  /** The callback to update the value. */
  update?: (newValue: Beam.Date) => void;
}

/** A date input that converts between Beam.Date and JavaScript Date. */
export class BeamDateInput extends React.Component<Properties> {
  public render(): JSX.Element {
    const year = String(this.props.value.year).padStart(4, '0');
    const month = String(this.props.value.month).padStart(2, '0');
    const day = String(this.props.value.day).padStart(2, '0');
    return <input type='date' value={`${year}-${month}-${day}`}
      onChange={this.onChange}/>;
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const parts = event.target.value.split('-');
    if(parts.length === 3) {
      this.props.update(new Beam.Date(
        parseInt(parts[0]), parseInt(parts[1]), parseInt(parts[2])));
    }
  }
}
