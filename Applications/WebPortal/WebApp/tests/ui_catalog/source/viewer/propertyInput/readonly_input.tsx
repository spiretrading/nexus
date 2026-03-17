import * as React from 'react'

interface Properties {

  /** The value of the field. */
  value?: any;
}

/** A readonly field field. */
export class ReadonlyInput extends React.Component<Properties> {
  public static readonly defaultProps: Partial<Properties> = {
    value: ''
  }

  public render(): JSX.Element {
    return <div>Edit Source Code</div>;
  }
}
