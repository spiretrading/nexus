import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { Interval } from '../models';
import { IntegerInput } from './integer_input';
import { Select } from './select';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'onChange'> {

  /** The interval to display. */
  value: Interval;

  /** Whether the interval can only be viewed. */
  readOnly?: boolean;

  /** Whether both controls are disabled. */
  disabled?: boolean;

  /** Whether the count must be filled in. */
  required?: boolean;

  /** Called when either field changes.
   * @param value - The updated interval, retaining the other field's value.
   */
  onChange?: (value: Interval) => void;
}

/** Edits a positive integer count and a calendar unit. */
export class IntervalInput extends React.Component<Properties> {
  public render(): JSX.Element {
    const {id, value, readOnly, disabled, required, onChange, className,
      ...rest} = this.props;
    return (
      <div role='group' {...rest}
          className={[css(STYLES.container), className].join(' ')}>
        <IntegerInput id={id} value={value.count} min={1} step={1}
          aria-invalid={rest['aria-invalid']}
          aria-describedby={rest['aria-describedby']}
          inputMode='numeric' aria-label='Interval count'
          readOnly={readOnly} disabled={disabled} required={required}
          style={{width: '64px', flex: '0 0 64px'}}
          onChange={this.onCountChange}/>
        <Select value={value.unit} aria-label='Interval unit'
            aria-invalid={rest['aria-invalid']}
            aria-describedby={rest['aria-describedby']}
            readOnly={readOnly} disabled={disabled}
            style={{flex: '1 1 0', minWidth: 0, boxSizing: 'border-box'}}
            onChange={this.onUnitChange}>
          {['Day', 'Week', 'Month', 'Year'].map((label, unit) => {
            if(value.count !== 1) {
              label += 's';
            }
            return <option key={unit} value={unit}>{label}</option>;
          })}
        </Select>
      </div>);
  }

  private onCountChange = (count: number) => {
    this.props.onChange?.(new Interval(count ?? null, this.props.value.unit));
  };

  private onUnitChange = (unit: string) => {
    this.props.onChange?.(new Interval(this.props.value.count, Number(unit)));
  };
}

const STYLES = StyleSheet.create({
  container: {display: 'flex', gap: '10px', width: '100%'}
});
