import * as React from 'react';
import { RadioButton } from 'web_portal';

interface Properties {

  /** The label of the first option. */
  label: string;

  /** The native group name shared by the options. */
  name: string;

  /** Whether the first option is selected. */
  checked: boolean;

  /** Whether the group is disabled. */
  disabled: boolean;

  /** Reports whether the first option is selected. */
  onChange?: (checked: boolean) => void;
}

/** Demonstrates mutually exclusive radio options and keyboard navigation. */
export class RadioButtonExample extends React.Component<Properties> {
  public render(): JSX.Element {
    return <div role='radiogroup' aria-label='Run report'
        style={{display: 'flex', flexDirection: 'column', gap: '10px'}}>
      <RadioButton label={this.props.label} name={this.props.name} value='now'
        checked={this.props.checked} disabled={this.props.disabled}
        onChange={this.onChange}/>
      <RadioButton label='On a Schedule' name={this.props.name}
        value='scheduled' checked={!this.props.checked}
        disabled={this.props.disabled} onChange={this.onChange}/>
    </div>;
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.props.onChange?.(event.target.value === 'now');
  };
}
