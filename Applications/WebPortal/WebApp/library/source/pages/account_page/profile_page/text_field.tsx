import * as React from 'react';

interface Properties {

  /** Indicates the input field can not be interacted with. */
  disabled?: boolean;

  /** The value to display in the field. */
  value?: string;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onInput?: (value: string) => void;
}

/** Displays a single text input field. */
export class TextField extends React.Component<Properties> {
  public static readonly defaultProps = {
    disabled: false,
    value: '',
    onInput: (_: string) => {}
  }

  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    return null;
  }
}
