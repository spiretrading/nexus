import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value?: Nexus.Security;

  /** The callback to update the value. */
  update?: (newValue: Nexus.Security) => void;
}

/** A text input that converts between string and Nexus.Security. */
export class SecurityInput extends React.Component<Properties> {
  public render(): JSX.Element {
    return <input value={this.props.value?.toString() ?? ''}
      onChange={this.onChange}/>;
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const parsed = Nexus.Security.parse(event.target.value.toUpperCase());
    if(!parsed.equals(Nexus.Security.NONE)) {
      this.props.update(parsed);
    }
  }
}
