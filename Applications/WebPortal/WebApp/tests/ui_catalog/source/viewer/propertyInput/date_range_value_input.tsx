import * as Beam from 'beam';
import * as React from 'react';
import { DateRange } from 'web_portal';

interface Properties {

  /** The range shown in the property editor. */
  value: DateRange;

  /** Updates the catalog's committed range. */
  update?: (value: DateRange) => void;
}

/** Edits the nullable bounds of a date range property. */
export class DateRangeValueInput extends React.Component<Properties> {
  public render(): JSX.Element {
    return <div style={{display: 'flex', flexDirection: 'column', gap: '4px'}}>
      <input type='date' aria-label='Start property'
        value={format(this.props.value.start)} onChange={this.onStartChange}/>
      <input type='date' aria-label='End property'
        value={format(this.props.value.end)} onChange={this.onEndChange}/>
    </div>;
  }

  private onStartChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.props.update?.(new DateRange(parse(event.target.value),
      this.props.value.end));
  };

  private onEndChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.props.update?.(new DateRange(this.props.value.start,
      parse(event.target.value)));
  };
}

function format(value: Beam.Date): string {
  if(value == null) {
    return '';
  }
  return `${String(value.year).padStart(4, '0')}-` +
    `${String(value.month).padStart(2, '0')}-` +
    String(value.day).padStart(2, '0');
}

function parse(value: string): Beam.Date {
  if(!value) {
    return null;
  }
  const [year, month, day] = value.split('-').map(Number);
  return new Beam.Date(year, month, day);
}
