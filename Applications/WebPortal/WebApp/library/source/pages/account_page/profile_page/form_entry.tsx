import * as React from 'react';

interface Properties {

  /** The name of the input field. */
  name: string;

  /** Whether the form is rendered vertically or horizontally. */
  orientation: FormEntry.Orientation;

  /** The input field to render. */
  children: JSX.Element;
}

/** Displays an input form for a single item. */
export class FormEntry extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    return null;
  }
}

export namespace FormEntry {

  /** Lists the ways that a form can be displayed. */
  export enum Orientation {

    /** The form is rendered horizontally. */
    HORIZONTAL,

    /** The form is rendered vertically. */
    VERTICAL
  }
}
