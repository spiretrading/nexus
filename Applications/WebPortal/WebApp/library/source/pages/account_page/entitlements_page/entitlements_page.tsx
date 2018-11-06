import * as React from 'react';

export class EntitlementsPage extends React.Component<{}, {}> {
  constructor(properties: {}) {
    super(properties);
  }

  public render(): JSX.Element {
    return <div/> ;
  }
}

export module EntitlementsPageSizing {
  export enum BreakPoint {

    /** Page is between 320 and 767 pixels (inclusive). */
    SMALL,

    /** Page is between 768 and 1035 pixels (inclusive). */
    MEDIUM,

    /** Page is bigger than 1036px. */
    LARGE,
  }
}
