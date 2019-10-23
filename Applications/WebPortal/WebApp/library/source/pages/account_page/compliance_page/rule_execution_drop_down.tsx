import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { DisplaySize } from '../../../display_size';

interface Properties {
  onChange?: () => void;
  value?: string;
}

export class RuleExecutionDropDown extends React.Component<Properties> {
  constructor() {
    super(null);
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {
    return (
      <select onChange={this.onChange} value={this.props.value}>
        <option value=''>{'Active'}</option>
        <option value=''>{'Active Per Account'}</option>
        <option value=''>{'Active Consolodated'}</option>
        <option value=''>{'Passive'}</option>
      </select>);
  }

  public onChange(event: React.ChangeEvent<HTMLSelectElement>): void {
  }
  public static readonly STYLE = {
  };
}
