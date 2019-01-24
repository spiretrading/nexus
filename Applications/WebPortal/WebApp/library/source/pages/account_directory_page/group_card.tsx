import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '../../display_size';
import { AccountEntry } from '.';

interface Properties {
  displaySize: DisplaySize;
  group: Beam.DirectoryEntry;
  accounts: AccountEntry[];
  filter: string;
}

export class GroupCard extends React.Component<Properties> {
    public render(): JSX.Element {
    return <div/>;
  }
}
