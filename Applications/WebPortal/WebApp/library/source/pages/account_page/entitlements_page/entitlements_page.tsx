import * as React from 'react';

enum BreakPointMode {
  MOBILE,
  DESKTOP
}

export class EntitlementsPage extends React.Component<{}, {}> {
  constructor(properties: {}) {
    super(properties);
  }

  public render(): JSX.Element {
    return <div/> ;
  }
}


export module EntitlementsPage {
  export const BreakPoint = BreakPointMode;
}