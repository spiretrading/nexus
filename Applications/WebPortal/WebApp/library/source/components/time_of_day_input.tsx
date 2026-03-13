import * as React from 'react';
import { DurationInput } from './duration_input';

type Properties = Omit<React.ComponentProps<typeof DurationInput>,
  'maxHourValue' | 'minHourValue'>;

/** A component that displays a time of day. */
export function TimeOfDayInput(props: Properties): JSX.Element {
  return <DurationInput {...props} maxHourValue={23}/>;
}
